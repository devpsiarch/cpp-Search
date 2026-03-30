#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include <algorithm>
#include <array>
#include <vector>
#include <functional>
#include <random>
#include <cmath>
#include <set>

namespace dtd {

class local_node {
    // the data state is left for thr deriver to impliment
public:

    friend struct MinimizeObjectiveFunction;
    friend struct MaximizeObjectiveFunction;

    virtual int objective_function() const noexcept = 0;
    virtual std::vector<dtd::local_node*> get_successors() const noexcept = 0;

    virtual void print() const noexcept = 0;

    virtual ~local_node() = default;
};

struct MinimizeObjectiveFunction {
    bool operator()(const dtd::local_node* lhs, const dtd::local_node* rhs) const {
        if (!lhs && !rhs) return false;
        if(!lhs) return true;
        if(!rhs) return false;
        if (lhs == rhs) return false;
        return lhs->objective_function() < rhs->objective_function();
    }
};

struct MaximizeObjectiveFunction {
    bool operator()(const dtd::local_node* lhs, const dtd::local_node* rhs) const {
        if (!lhs && !rhs) return false;
        if(!lhs) return true;
        if(!rhs) return false;
        if (lhs == rhs) return false;
        return lhs->objective_function() < rhs->objective_function();
    }
};

template <typename Behavior = dtd::MinimizeObjectiveFunction>
dtd::local_node* hill_climbing_algorithm(dtd::local_node* initial_node,size_t limit = 10){
    dtd::local_node* best_so_far = initial_node;
    while(limit > 0){

        std::vector<dtd::local_node*> successors = best_so_far->get_successors();
        successors.push_back(best_so_far);

        std::sort(successors.begin(),successors.end(),Behavior());

        best_so_far = successors[0];

        // cleanup part
        for(int i = 1 ; i < successors.size() ; i++){
            successors[i]->print();
            delete successors[i];
        }

        limit--;
    }
    return best_so_far;
}


template <typename Behavior = dtd::MinimizeObjectiveFunction>
dtd::local_node* stochastic_hill_climbing_algorithm(
    dtd::local_node* initial_node,
    std::function<std::vector<float>(const std::vector<dtd::local_node*>)> get_weights,
    size_t limit = 100
){
    dtd::local_node* best_so_far = initial_node;

    static std::random_device _rnd{};
    static std::mt19937 _engine{_rnd()};
    std::uniform_real_distribution<float> _norm{0.0f,1.0f}; // we use it a lot


    while(limit > 0){

        std::vector<dtd::local_node*> successors = best_so_far->get_successors();
        successors.push_back(best_so_far);

        std::sort(successors.begin(),successors.end(),Behavior());

        std::vector<float> weights = get_weights(successors);

        float gotten = _norm(_engine);
        float current = 0.f;
        
        best_so_far = nullptr;

        std::discrete_distribution<size_t> dist(weights.begin(),weights.end());

        best_so_far = successors[dist(_engine)];

        // cleanup part
        for(int i = 0 ; i < successors.size() ; i++){
            if(successors[i] != best_so_far){
                // successors[i]->print();
                delete successors[i];
            }
        }

        limit--;
    }
    return best_so_far;
}

template <typename Behavior = dtd::MinimizeObjectiveFunction>
dtd::local_node* random_restart_hill_climbing(std::vector<dtd::local_node*>& initial_nodes){
    std::vector<dtd::local_node*> best_of_the_best(initial_nodes.size(),nullptr);
    
    for(unsigned int i = 0 ; i < initial_nodes.size() ; i++){
        best_of_the_best[i] = hill_climbing_algorithm(initial_nodes[i]);
    }

    std::sort(best_of_the_best.begin(),best_of_the_best.end(),Behavior());
    // cleanup part
    for(int i = 1 ; i < best_of_the_best.size() ; i++){
        best_of_the_best[i]->print();
        delete best_of_the_best[i];
    }
    return best_of_the_best[0];
}

template <typename Policy = std::less_equal<int>>
dtd::local_node* first_choice_hill_climbing(dtd::local_node* inital_node,std::function<dtd::local_node*()> get_random_move,int limit = 50){
    Policy cmp;

    dtd::local_node* best_so_far = inital_node;
    int best_cost_so_far = inital_node->objective_function();
    while(limit > 0){

        dtd::local_node* candidate = get_random_move();
        int candidate_cost = candidate->objective_function();

        while(!cmp(candidate_cost,best_cost_so_far)){
            delete candidate;
            candidate = get_random_move();
            candidate_cost = candidate->objective_function();
        }

        std::swap(best_so_far,candidate);
        std::swap(best_cost_so_far,candidate_cost);

        delete candidate;

        limit--;
    }

    return best_so_far;
}


// here we define the schedule type that encodes the cooling sequence during simulated aneaaling

class cooling_schedule {
private:
    // the inital temperature is set by the deriving class
public:
    virtual float get_instance(int t) const noexcept = 0;

    virtual ~cooling_schedule() = default;
};

class linear_cooling : public cooling_schedule {
private:
    float T0;
    float beta;
public:
    linear_cooling(float init,float beta_factor) : T0(init)  , beta(beta_factor){}
    ~linear_cooling() = default;
    virtual float get_instance(int t) const noexcept override {
        return T0 - beta*t;
    }
};

class exponential_cooling : public cooling_schedule {
private:
    float T0;
    float alpha;
public:
    exponential_cooling(float init,float alpha_factor) : T0(init)  , alpha(alpha_factor){}
    ~exponential_cooling() = default;
    virtual float get_instance(int t) const noexcept override {
        return T0 - alpha*t;
    }
};


template <typename CoolingStrat,typename Policy = std::less_equal<int>>
dtd::local_node* simulated_annealing(dtd::local_node* init,float init_t,float init_factor,int limit = 100){
    dtd::local_node* best_so_far = init;
    int best_cost_so_far = best_so_far->objective_function();

    CoolingStrat cooler(init_t,init_factor);
    Policy cmp;

    static std::random_device _rnd{};
    static std::mt19937 _engine{_rnd()};
    std::uniform_int_distribution<int> _norm;
    std::uniform_real_distribution<float> _real_norm(0.0f,1.0f);

    for(int t = 1 ; t < limit ; t++){
        int T = cooler.get_instance(t);
        
        if(T <= 0) return best_so_far;

        std::vector<dtd::local_node*> successors = best_so_far->get_successors();
        dtd::local_node* next = successors[_norm(_engine) % successors.size()];

        // cleanup the rest (except the next)
        for(unsigned int i = 0 ; i < successors.size() ; i++){
            if(successors[i] == next) continue;
            delete successors[i];
        }

        int next_cost = next->objective_function();

        int Delta_energy = next_cost - best_cost_so_far;

        if(!cmp(Delta_energy,0)){
            std::swap(best_so_far,next);
            delete next;
            best_cost_so_far = next_cost;
        }else{
            float threash = std::expf((float)Delta_energy / (float)T);
            float gotten = _real_norm(_engine);
            // if we landed in the probability 
            if(gotten < threash){
                std::swap(best_so_far,next);
                delete next;
                best_cost_so_far = next_cost;
            }else{
                delete next;
            }
        }
    }

    return best_so_far;
}

template <typename Behavior = dtd::MinimizeObjectiveFunction,size_t BeamWidth>
dtd::local_node* beam_search(dtd::local_node* initial_node,size_t limit = 50){
    
    std::array<dtd::local_node*,BeamWidth> beam{};
    beam[0] = initial_node;

    while(limit > 0){

        std::vector<dtd::local_node*> all_nodes;

        for(size_t i = 0 ; i < BeamWidth ; i++){
            if(!beam[i]) continue;

            std::vector<dtd::local_node*> successors = beam[i]->get_successors();
            for(size_t j = 0 ; j < successors.size() ; j++) all_nodes.push_back(successors[j]);
            delete beam[i];
        }

        std::sort(all_nodes.begin(),all_nodes.end(),Behavior());

        size_t i;

        for(i = 0 ; i < BeamWidth && i < all_nodes.size() ; i++){
            beam[i] = all_nodes[i];
        }

        // if we ran out of beams
        for(size_t j = i; j < BeamWidth; j++) {
            beam[j] = nullptr;
        }

        // we delete the pruned nodes (not the best)
        for(; i < all_nodes.size() ; i++){
            if(!all_nodes[i]) continue;

            delete all_nodes[i];
        }
    
        if(!beam[0]) break;
        limit--;
    }
    // return the best ever from the beam
    dtd::local_node* best_ever = beam[0];
    for(size_t i = 1 ; i < BeamWidth ; i++){
        if(!beam[i]) continue;

        delete beam[i];
    }
    return best_ever;
}


template <typename Behavior = dtd::MinimizeObjectiveFunction,size_t BeamWidth>
dtd::local_node* stochastic_beam_search(
        dtd::local_node* initial_node,
        std::function<std::vector<float>(const std::vector<dtd::local_node*>)> get_weights,
        size_t limit = 50
    ){
    
    static std::random_device _rnd{};
    static std::mt19937 _engine{_rnd()};
    std::uniform_real_distribution<float> _norm{0.0f,1.0f};

    std::array<dtd::local_node*,BeamWidth> beam{};
    beam[0] = initial_node;


    while(limit > 0){

        std::vector<dtd::local_node*> all_nodes;

        for(size_t i = 0 ; i < BeamWidth ; i++){
            if(!beam[i]) continue;

            std::vector<dtd::local_node*> successors = beam[i]->get_successors();
            for(size_t j = 0 ; j < successors.size() ; j++) all_nodes.push_back(successors[j]);
            delete beam[i];
            beam[i] = nullptr;
        }

        std::sort(all_nodes.begin(),all_nodes.end(),Behavior());
        std::vector<float> weights = get_weights(all_nodes);

        std::set<size_t> chosen_indexes;

        size_t mn_len = std::min(BeamWidth,all_nodes.size());
        for(size_t i = 0 ; i < BeamWidth ; i++){
            std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
            size_t chosen_index = dist(_engine);

            beam[i] = all_nodes[chosen_index];
            weights[chosen_index] = 0.0f;
            chosen_indexes.insert(chosen_index);
        }

        // cleanup 
        for(int i = 0 ; i < all_nodes.size() ; i++){
            if(chosen_indexes.find(i) != chosen_indexes.end()) continue;
            delete all_nodes[i];
        }
        limit--;
    }
    // return one of them based on a discrete_distribution
    dtd::local_node* best_ever = nullptr;
    
    std::vector<dtd::local_node*> nodes;
    
    for(auto ptr:beam) nodes.push_back(ptr);

    std::sort(nodes.begin(),nodes.end(),Behavior());
    std::vector<float> weights = get_weights(nodes);

    std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
    
    size_t chosen = dist(_engine);
    best_ever = nodes[chosen];

    // cleanup 
    for(int i = 0 ; i < BeamWidth ; i++){
        if(!nodes[i] || i == chosen) continue;
        delete nodes[i];
    }

    return best_ever;
}

}; // namespace dtd
#endif
