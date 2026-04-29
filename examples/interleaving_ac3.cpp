#include <iostream>
#include "../include/csp.hpp"

template <typename T>
struct diff : public dtd::Constraint<T> {
    virtual bool is_satisfied(const dtd::Assignment<T>& values) const noexcept override {
        if(values.find(this->x1) == values.end() || values.find(this->x2) == values.end()) return true;
        return values.at(this->x1) != values.at(this->x2);
    }
    virtual bool is_satisfied(T xi,T xj) const noexcept override {
        return xi != xj;
    }
    diff(const char* _x1,const char* _x2) : dtd::Constraint<T>(_x1,_x2) {}
};

template <typename T>
struct greater : public dtd::Constraint<T> {
    virtual bool is_satisfied(const dtd::Assignment<T>& values) const noexcept override {
        if(values.find(this->x1) == values.end() || values.find(this->x2) == values.end()) return true;
        return values.at(this->x1) > values.at(this->x2);
    }
    virtual bool is_satisfied(T xi,T xj) const noexcept override {
        return xi > xj;
    }
    greater(const char* _x1,const char* _x2) : dtd::Constraint<T>(_x1,_x2) {}
};

template <typename T>
struct lesser : public dtd::Constraint<T> {
    virtual bool is_satisfied(const dtd::Assignment<T>& values) const noexcept override {
        if(values.find(this->x1) == values.end() || values.find(this->x2) == values.end()) return true;
        return values.at(this->x1) < values.at(this->x2);
    }
    virtual bool is_satisfied(T xi,T xj) const noexcept override {
        return xi < xj;
    }
    lesser(const char* _x1,const char* _x2) : dtd::Constraint<T>(_x1,_x2) {}
};

template <typename T>
struct equal : public dtd::Constraint<T> {
    virtual bool is_satisfied(const dtd::Assignment<T>& values) const noexcept override {
        if(values.find(this->x1) == values.end() || values.find(this->x2) == values.end()) return true;
        return values.at(this->x1) == values.at(this->x2);
    }
    virtual bool is_satisfied(T xi,T xj) const noexcept override {
        return xi == xj;
    }
    equal(const char* _x1,const char* _x2) : dtd::Constraint<T>(_x1,_x2) {}
};

int main(void){

    // Define the variables (Territories)
    std::vector<const char*> variables = {
        "B", "A","C"
    };

    // Define the domain for each variable (3 colors: 1, 2, 3)
    std::vector<std::set<int>> domains = {
        {1, 2, 3}, // B
        {1, 2, 3}, // A 
        {1, 2, 3}, // C
    };

    // Initialize the CSP
    dtd::CSP<int> Problem(variables, domains);

    // Define the constraints (Neighboring regions must have different colors)
    dtd::ConstraintChecker<int> vendita;

    // Constraints based on the map of Australia:
    vendita.constraints.push_back(new equal<int>("B", "C"));
    vendita.constraints.push_back(new greater<int>("A", "B"));
    vendita.constraints.push_back(new equal<int>("C", "B"));
    vendita.constraints.push_back(new lesser<int>("B", "A"));

    // Lets run Arc consistency (wont do much in this example)
    // if(!dtd::AC_3(Problem, vendita)){
    //     std::cout << "Problem is not ARC CONSISTENT\n";
    //     return 0;
    // }else{
    //     std::cout << "Problem is ARC CONSISTENT\n";
    // }

    for(const char* name:Problem.variables){
        size_t index = Problem.domain_index[name];
        std::cout << "Allowed values for " << name << " are:";
        for(auto v:Problem.domains[index]){
            std::cout << v << " ";
        }
        std::cout << '\n';
    }

    // Search for solutions
    std::vector<dtd::Assignment<int>> solutions;
    dtd::Assignment<int> current_assignment;
    dtd::OnlineAssignment<int> p(&Problem,&vendita);

    dtd::backtracking_search_interleaving_inference(p,0,solutions);
    // dtd::backtracking_search(Problem,vendita,0,current_assignment,solutions);

    // Output results
    std::cout << "Backtrack found " << solutions.size() << " valid colorings." << std::endl;

    if (!solutions.empty()) {
        std::cout << "Example solution (First match):" << std::endl;
        for(auto ass:solutions){
            for (const auto& pair : ass) {
                std::cout << pair.first << " -> Color " << pair.second << " | ";
            }
            std::cout << std::endl;
        }
    }

    return 0;
}
