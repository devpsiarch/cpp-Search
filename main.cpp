#include <iostream>
#include "./include/csp.hpp"

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

int main(void){

    // Define the variables (Territories)
    std::vector<const char*> variables = {
        "WA", "NT", "SA", "Q", "NSW", "V", "T"
    };

    // Define the domain for each variable (3 colors: 1, 2, 3)
    std::vector<std::set<int>> domains = {
        {1, 2, 3}, // WA
        {1, 2, 3}, // NT
        {1, 2, 3}, // SA
        {1, 2, 3}, // Q
        {1, 2, 3}, // NSW
        {1, 2, 3}, // V
        {1, 2, 3}  // T
    };

    // Initialize the CSP
    dtd::CSP<int> australia(variables, domains);

    // Define the constraints (Neighboring regions must have different colors)
    dtd::ConstraintChecker<int> aust_c;

    // Constraints based on the map of Australia:
    aust_c.constraints.push_back(new diff<int>("WA", "NT"));
    aust_c.constraints.push_back(new diff<int>("WA", "SA"));
    aust_c.constraints.push_back(new diff<int>("NT", "SA"));
    aust_c.constraints.push_back(new diff<int>("NT", "Q"));
    aust_c.constraints.push_back(new diff<int>("SA", "Q"));
    aust_c.constraints.push_back(new diff<int>("SA", "NSW"));
    aust_c.constraints.push_back(new diff<int>("SA", "V"));
    aust_c.constraints.push_back(new diff<int>("Q", "NSW"));
    aust_c.constraints.push_back(new diff<int>("NSW", "V"));

    // Tasmania (T) has no neighbors on the mainland, so no diff constraints needed unless 
    // you want to force it to be different from a specific state for some reason.

    // Lets run Arc consistency (wont do much in this example)
    if(!dtd::AC_3(australia, aust_c)){
        std::cout << "Problem is not ARC CONSISTENT\n";
        return 0;
    }else{
        std::cout << "Problem is ARC CONSISTENT\n";
    }

    // Search for solutions
    std::vector<dtd::Assignment<int>> solutions;
    dtd::Assignment<int> current_assignment;

    dtd::backtracking_search(australia, aust_c, 0, current_assignment, solutions);

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
