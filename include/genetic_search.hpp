#ifndef GENETIC_SEARCH_H
#define GENETIC_SEARCH_H

#include "local_search.hpp"

namespace dtd {

// abstract class to define the API for a genetic state 
// to be used by different genetic algorithms 
class genetic_state : public dtd::local_node {
public:
    

    virtual std::vector<dtd::local_node*> get_successors() const noexcept = 0;

    virtual void mutate(float probability = 0.5f) noexcept = 0;
    virtual genetic_state* cross_over(genetic_state* other) const noexcept = 0;



    virtual float fittness_function() const noexcept final {
        return this->objective_function();
    }


};

template <typename Behavior = dtd::MinimizeObjectiveFunction>
dtd::genetic_state* simple_genetic_algorithm(
    std::vector<dtd::genetic_state*>& population,
    float mutation_probability,
    std::function<std::vector<int>(const std::vector<dtd::genetic_state*>&)> generate_weights,
    std::function<dtd::genetic_state*(const std::vector<dtd::genetic_state*>&)> pick_one,
    int limit = 10
    
){
    static std::random_device _rnd{};
    static std::mt19937 _engine{_rnd()};

    // here i move since i want the user to not use the passed array anymore (not sure)
    std::vector<dtd::genetic_state*> current_population = std::move(population);
    while(limit > 0){
        if(current_population.size() == 1) break;
        
        std::vector<int> weights = generate_weights(current_population);

        std::discrete_distribution<size_t> dist(weights.begin(), weights.end());

        std::vector<dtd::genetic_state*> next_population;

        for(size_t i = 0 ; i < current_population.size() ; i++){
            auto x = dist(_engine);
            weights[x] = 0;
            auto y = dist(_engine);

            auto father = current_population[x];
            auto mother = current_population[y];

            auto son = father->cross_over(mother);

            son->mutate(mutation_probability);

            next_population.push_back(son);
        }

        // cleanup the old population 
        for(auto ptr:current_population)
            delete ptr; 
        // i think here , a copy is faster then a move (its just pointers)
        current_population = next_population;


        std::sort(current_population.begin(),current_population.end());
        limit--;
    }
    dtd::genetic_state* best_so_far = pick_one(current_population);
    // cleanup 
    for(auto ptr:current_population)
        if(ptr != best_so_far) delete ptr;
    return best_so_far;
}


};

#endif
