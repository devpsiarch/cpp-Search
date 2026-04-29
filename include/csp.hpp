#ifndef CSP_H
#define CSP_H

#include <limits>
#include <queue>
#include <set>
#include <stack>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

#define panic(msg)                \
do {                              \
    printf("[PANIC]: %s.\n",msg); \
    exit(1);                      \
} while(0);                       

namespace dtd {

using Name = const char*;
template <typename T>
using Assignment = std::unordered_map<Name, T>; 

namespace Option {
    enum class Inference {
        MAC,FC,None
    };
    enum class VariableOrdering {
        MRV,DH,None
    };
    enum class ValueOrdering {
        LCV,None
    };
};

// this is the problem class that the users will have 
// to enherite from to define there problem statement
template <typename T>
class CSP {
public:
    // defines the names of the variables that we need to 
    // assign values to
    std::vector<Name> variables;
    // the domains variables either expresses the possible values 
    // of the illegal values based on the implimentation of the user
    std::vector<std::set<T>> domains;
    
    std::unordered_map<Name,size_t> domain_index;

    CSP(std::vector<Name> variables,std::vector<std::set<T>> domains) :
        variables(variables) , domains(domains){
            // we associate every variables with its domain for future refrence
            for(size_t i = 0 ; i < variables.size() ; i++){
                domain_index[variables[i]] = i;
            }
        }



};


// encodes only binary constraints so far
// maybe well add more constraint type in the future
//
// we define two is_satisfied function for both cases 
// of passing a entire Assignment and if we just wanna 
// check the indivisual constriant for scope
template <typename T>
struct Constraint {
    const char* x1;
    const char* x2;
    virtual bool is_satisfied(const Assignment<T>& values) const noexcept = 0;
    virtual bool is_satisfied(T xi,T xj) const noexcept = 0;
    Constraint(const char* _x1,const char* _x2) : x1(_x1), x2(_x2){}
    virtual ~Constraint() = default;
};

// a struct that the use shall use to pack all the constraints 
// of the problem together in one place.
// the constraintCheck OWNS the contraint for the lifetime of the 
// problem solving (deleted copies for now)
template <typename T>
struct ConstraintChecker {
    std::vector<Constraint<T>*> constraints;

    ConstraintChecker() = default;

    ~ConstraintChecker() {
        for(const Constraint<T>* ptr : constraints)
            delete ptr;
    }

    ConstraintChecker(const ConstraintChecker& other) = delete;
    ConstraintChecker& operator=(const ConstraintChecker& other) = delete;

    ConstraintChecker(ConstraintChecker&& other) noexcept 
        : constraints(std::move(other.constraints)) {
    }

    ConstraintChecker& operator=(ConstraintChecker&& other) noexcept {
        if (this == &other) return *this;

        // Free current resources
        for(const Constraint<T>* ptr : constraints) {
            delete ptr;
        }

        // Steal resources from other
        constraints = std::move(other.constraints);

        return *this;
    }

    bool is_valid_assignment(const Assignment<T>& values) const noexcept {
        for(const Constraint<T>* ptr : constraints)
            if(!ptr->is_satisfied(values)) return false;

        return true;
    }

    // returns non-owning pointers to the constriant DO NOT DELETE
    std::vector<Constraint<T>*> get_constriants() const noexcept {
        return this->constraints;
    }

    // returns all the constraints that have a relation to x2 = xj
    std::vector<Constraint<T>*> get_constriants_x2(std::string rel,std::string except) const noexcept{
        std::vector<Constraint<T>*> ans;
        for(auto ptr:this->constraints){
            auto xk = std::string(ptr->x2);
            if(xk == rel && xk != except) ans.push_back(ptr);
        }
        return ans;
    }
};




// a dynamic representation of a Assignment
// it owns the set of ever chaning domains 
// that correspond to each variable
//
// the handling of remove and re-add later 
// is handle by this class and not exepected from 
// any other inference algorithm
template <typename T>
struct OnlineAssignment {

    size_t get_minimum_remaining_value(const char* xi) const {
        return live_domains.top()[this->problem->domain_index.at(xi)].size();
    }

    struct MRVComparator {
        const OnlineAssignment* parent; // Pointer to the main class instance
        
        MRVComparator(OnlineAssignment* p) : parent(p) {}

        // The priority_queue uses this operator to compare elements
        bool operator()(const char* a, const char* b) const {
            return parent->get_minimum_remaining_value(a) > parent->get_minimum_remaining_value(b);
        }
    };


    const CSP<T>* problem;
    Assignment<T> values;
    size_t number_of_assigned_values;

    std::stack<std::vector<std::set<T>>> live_domains;

    // maps each variable name to the number 
    // of variables its related too 
    // used in variables ordering
    std::unordered_map<const char*,size_t> degree_map;

    // we refrence a problen and keep it with us 
    // we will not effect nore delete this problem
    // used for variable ordering
    std::priority_queue<const char*,std::vector<const char*>,MRVComparator> assign_queue;

    // copy the set of domains from the problen statment
    OnlineAssignment(const CSP<T>* problem,const ConstraintChecker<T>& checker)
        : problem(problem) , values(), number_of_assigned_values(0) , assign_queue(MRVComparator(this)){
        live_domains.push(problem->domains);
        for(size_t i = 0 ; i < this->problem->variables.size() ; i++){
            assign_queue.push(this->problem->variables[i]);
        }
        
        // populate degree_map member variables the contains the 
        // number of tied variables to another by a constraint 
        // used to variable ordering
        
        // init to 0 before anything happens
        for(size_t i = 0 ; i < this->problem->variables.size() ; i++){
            this->degree_map[this->problem->variables[i]] = 0;
        }
        // populate
        std::vector<Constraint<T>*> const_list = checker.get_constriants();
        for(size_t i = 0 ; i < const_list.size() ; i++){
            this->degree_map[const_list[i]->x1]++;
        }
    }


    // Main function to call during the backtracking search to perform 
    // the variable orderig under some policy, return index of the next 
    // variable to assign
    size_t get_next_variable(Option::VariableOrdering option) const noexcept {
        switch (option) {
            case Option::VariableOrdering::MRV:{
                int variable_index = 0;
                size_t min_domain = std::numeric_limits<size_t>::max();
                for(size_t i = 0 ; i < this->problem->variables.size() ; i++){
                    const char* name = this->problem->variables[i];
                    if(this->values.find(name) != this->values.end()) continue;
                    if(this->live_domains.top()[i].size() < min_domain){
                        min_domain = this->live_domains.top()[i].size();
                        variable_index = i;
                    }
                }
                return variable_index;
                break;
            }
            case Option::VariableOrdering::DH:{
                int variable_index = 0;
                size_t max_domain = std::numeric_limits<size_t>::min();
                for(size_t i = 0 ; i < this->problem->variables.size() ; i++){
                    const char* name = this->problem->variables[i];
                    if(this->values.find(name) != this->values.end()) continue;
                    if(this->degree_map.at(name) > max_domain){
                        max_domain = this->live_domains.top()[i].size();
                        variable_index = i;
                    }
                }
                return variable_index;               
                break;
            }
            case Option::VariableOrdering::None:
                return this->number_of_assigned_values;
                break;
        }
    }

    // Main function called during the backtracking_search to perform 
    // value ordering, we return a vector (for now a set) of values T ordered by a custom 
    // ordering function
    std::set<T> get_next_values(size_t index,Option::ValueOrdering option) const noexcept {
        switch (option) {
            case Option::ValueOrdering::LCV:
                panic("LCV not implimented yet");
                break;
            case Option::ValueOrdering::None:
                return live_domains.top()[index];
                break;
        }
    }

    
    bool is_complete_assignment() const noexcept {
        return this->number_of_assigned_values == this->problem->variables.size();
    }

};

// some debug and printing function to make sure the 
// algorithm are working properly 
template <typename T>
void inspect_live_domain(const OnlineAssignment<T>& assigned){
    std::cout << "[INSPECT SEQUENCE]:\n";
    for(const char* name:assigned.problem->variables){
        size_t index = assigned.problem->domain_index.at(name);
        std::cout << "D:" << name << " are:";
        for(auto v:assigned.live_domains.top().at(index)){
            std::cout << v << " ";
        }
        std::cout << '\n';
    }
    std::cout << "[INSPECT SEQUENCE].\n";
}


template <typename T>
void backtracking_search(const CSP<T>& problem,
        const ConstraintChecker<T>& checker,
        size_t index,
        Assignment<T>& values,
        std::vector<Assignment<T>>& all_solutions)
{ 
    // solution found
    if(index >= problem.variables.size()){
        all_solutions.push_back(values);
        return;
    }

    auto current_variable = problem.variables[index];

    for(auto value:problem.domains[index]){
        values[current_variable] = value;
        
        if(checker.is_valid_assignment(values) == false){
            goto defer;
        }

        backtracking_search(problem,checker,index+1,values,all_solutions);

        defer:
            values.erase(current_variable);
    } 
}

// Arc consistency inference algorithm will alter a given problem statement to 
// insure the consistency , this should never be called from a interleaving 
// search since it ALTERS THE WHOLE PROBLEM FOREVER , a dedicated interleaving function 
// with reversable functions will be implimented for interleaving 
//
// this should be called before the search begins for solving a given CSP.
template <typename T>
bool AC_3(CSP<T>& problem,const ConstraintChecker<T>& checker){
    std::vector<Constraint<T>*> agenda = checker.get_constriants();
    while(!agenda.empty()){
        Constraint<T>* current_const = agenda.back();
        agenda.pop_back();
        if(revise_AC_3(problem, current_const)){
            // check if a variable has empty set domain, thus cannot be solved
            if(problem.domains[problem.domain_index[current_const->x1]].size() <= 0) return false;

            auto new_constraints = checker.get_constriants_x2(current_const->x1,current_const->x2);
            // add the new updated constraint to the agenda
            for(auto ptr:new_constraints){
                agenda.push_back(ptr);
            }
        }
    }
    return true;
}

template <typename T>
bool revise_AC_3(CSP<T>& problem,const Constraint<T>* current_const){
    bool domain_changed = false;
    size_t x1_domain = problem.domain_index[current_const->x1];
    size_t x2_domain = problem.domain_index[current_const->x2];

    for(T value1:problem.domains[x1_domain]){
        bool satisfying_value_found = false;
        for(T value2:problem.domains[x2_domain]){
            if(current_const->is_satisfied(value1,value2)){
                satisfying_value_found = true;
            }
        }
        // if we could not found a value2 that satisfies the constraint 
        // then we remove value1 from its domain
        if(!satisfying_value_found){
            problem.domains[x1_domain].erase(value1);
            domain_changed = true;
        }
    }
    return domain_changed;
}

// Forward Checking for Arc consistency
template <typename T>
bool FC(OnlineAssignment<T>& assigned,const ConstraintChecker<T>& checker){
    std::vector<Constraint<T>*> agenda = checker.get_constriants();

    // push a copy of the current domain on the stack 
    assigned.live_domains.push(assigned.live_domains.top());

    while(!agenda.empty()){
        
        Constraint<T>* current_const = agenda.back();
        agenda.pop_back();

        size_t xi = assigned.problem->domain_index.at(current_const->x1);

        if(revise_inference(assigned, current_const)){
            // check if a variable has empty set domain, thus cannot be solved
            if(assigned.live_domains.top()[xi].size() <= 0) return false;

            auto new_constraints = checker.get_constriants_x2(current_const->x1,current_const->x2);
            // add the new updated constraint to the agenda
            for(auto ptr:new_constraints){
                agenda.push_back(ptr);
            }
        }
    }
    return true;
}


// Maintaining Arc consistency
template <typename T>
bool MAC(OnlineAssignment<T>& assigned,const ConstraintChecker<T>& checker){
    std::vector<Constraint<T>*> agenda = checker.get_constriants();

    // push a copy of the current domain on the stack 
    assigned.live_domains.push(assigned.live_domains.top());

    while(!agenda.empty()){
        
        Constraint<T>* current_const = agenda.back();
        agenda.pop_back();

        size_t xi = assigned.problem->domain_index.at(current_const->x1);

        if(revise_inference(assigned, current_const)){
            // check if a variable has empty set domain, thus cannot be solved
            if(assigned.live_domains.top()[xi].size() <= 0) return false;

            // we dont push new constraint into the agenda 
            // like we did in the AC-3 algorithm
        }
    }
    return true;
}

template <typename T>
bool revise_inference(OnlineAssignment<T>&assigned,const Constraint<T>* current_const){
    bool domain_changed = false;
    size_t x1_domain = assigned.problem->domain_index.at(current_const->x1);
    size_t x2_domain = assigned.problem->domain_index.at(current_const->x2);

    for(T value1:assigned.live_domains.top()[x1_domain]){
        bool satisfying_value_found = false;
        for(T value2:assigned.live_domains.top()[x2_domain]){
            if(current_const->is_satisfied(value1,value2)){
                satisfying_value_found = true;
            }
        }
        // if we could not found a value2 that satisfies the constraint 
        // then we remove value1 from its domain
        if(!satisfying_value_found){
            assigned.live_domains.top()[x1_domain].erase(value1);
            domain_changed = true;
        }
    }
    return domain_changed;
}


template <typename T>
void backtracking_search_interleaving_inference(
        OnlineAssignment<T>& assigned,
        const ConstraintChecker<T>& checker,
        size_t index,
        std::vector<Assignment<T>>& all_solutions,
        Option::Inference strategy = Option::Inference::None,
        Option::VariableOrdering varOrdering = Option::VariableOrdering::None,
        Option::ValueOrdering valOrdering = Option::ValueOrdering::None
        )
{ 
    // solution found (still sequential without variable selection)
    if(assigned.is_complete_assignment()){
        all_solutions.push_back(assigned.values);
        return;
    }

    auto current_variable = assigned.problem->variables[index];

    std::set<T> orderd_values = assigned.get_next_values(index,valOrdering);

    for(auto value:orderd_values){
        assigned.values[current_variable] = value;
        assigned.number_of_assigned_values++;

        // interleaving call
        switch (strategy){
            case Option::Inference::FC:
                FC(assigned, checker);
                break;
            case Option::Inference::MAC:
                MAC(assigned,checker);
                break;
            case Option::Inference::None:
                // Do not inference the domain
                break;
        }
        // inspect_live_domain(assigned);

        if(checker.is_valid_assignment(assigned.values) == false){
            goto defer;
        }


        backtracking_search_interleaving_inference(
            assigned,
            checker,
            assigned.get_next_variable(varOrdering),
            all_solutions,
            strategy,
            varOrdering,
            valOrdering
        );

        defer:
            // un-interleaver here
            assigned.values.erase(current_variable);
            assigned.number_of_assigned_values--;
            switch (strategy){
                case Option::Inference::None:
                    // If we used no inference , then 
                    // not fram was pushed to the stack then we do nothing
                    break;
                default:
                    // else if we used any inference strategy , we have to pop
                    // fram from the stack to get the previous live_domains
                    assigned.live_domains.pop();
                    break;
            }
            // inspect_live_domain(assigned);
    } 
}


};  // end of namespace

#endif
