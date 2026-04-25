#ifndef CSP_H
#define CSP_H

#include <queue>
#include <set>
#include <unordered_map>
#include <vector>
#include <string>

namespace dtd {


using Name = const char*;
template <typename T>
using Assignment = std::unordered_map<Name, T>; 

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

};  // end of namespace

#endif
