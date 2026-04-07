#ifndef SA_SEARCH_H
#define SA_SEARCH_H

#include <vector>
#include <limits>
#include <iostream>

namespace dtd {

class sa_node {
private:
    // private data is implimented by the deriving class
public:
    enum Behavior {
        LUCK,
        MIN,
        MAX
    };

    Behavior behavior;

    sa_node(Behavior b) : behavior(b){}

    virtual double evaluate() const noexcept = 0;
    virtual bool terminal_test() const noexcept = 0;
    
    virtual std::vector<std::pair<dtd::sa_node*,double>> expand_luck_outcome() const noexcept = 0;
    virtual std::vector<dtd::sa_node*> generate_successors() const noexcept = 0;

    virtual void print() const noexcept = 0;

    virtual ~sa_node() = default;
};


// stochastic algorithms

double sa_minimax_search(dtd::sa_node* init,unsigned int depth){
    if(depth == 0 || init->terminal_test()) return init->evaluate();

    init->print();

    double node_eval;

    switch(init->behavior){
        case dtd::sa_node::Behavior::MAX:{
            std::vector<dtd::sa_node*> adv_nodes = init->generate_successors();
            if(adv_nodes.size() == 0) return init->evaluate();

            double max_eval = -std::numeric_limits<double>::infinity();
            for(size_t i = 0 ; i < adv_nodes.size() ; i++){
                double eval = sa_minimax_search(adv_nodes[i],depth-1);
                max_eval = std::max(max_eval,eval);
            }
            node_eval = max_eval;

            for(auto ptr:adv_nodes) delete ptr;
            return node_eval;
            break;
        }
        case dtd::sa_node::Behavior::MIN:{
            std::vector<dtd::sa_node*> adv_nodes = init->generate_successors();
            if(adv_nodes.size() == 0) return init->evaluate();

            double min_eval = std::numeric_limits<double>::infinity();
            for(size_t i = 0 ; i < adv_nodes.size() ; i++){
                double eval = sa_minimax_search(adv_nodes[i],depth-1);
                min_eval = std::min(min_eval,eval);
            }
            node_eval = min_eval;

            for(auto ptr:adv_nodes) delete ptr;
            return node_eval;
            break;
        }
        case dtd::sa_node::Behavior::LUCK:{
            std::vector<std::pair<dtd::sa_node*,double>> node_proba = init->expand_luck_outcome();
            double weighted_eval = 0.0f;
            for(size_t i = 0 ; i < node_proba.size(); i++){
                // return the weighted sum of probability of event mult by evaluation of that subtree
                weighted_eval += sa_minimax_search(node_proba[i].first,depth-1)*node_proba[i].second;
            }
            node_eval = weighted_eval;
    
            std::cout << "calculated value: " << node_eval << '\n';

            for(auto ptr:node_proba) delete ptr.first;
            return node_eval;
            break;
        }
    }
}


dtd::sa_node* sa_minimax_decision(dtd::sa_node* init,unsigned int depth = 3){
    dtd::sa_node* best_move = nullptr;
    

    std::vector<dtd::sa_node*> adv_nodes = init->generate_successors();

    if(adv_nodes.size() == 0)
        return best_move;
    
    switch(init->behavior){
        case dtd::sa_node::Behavior::MAX:{
            double best_eval = -std::numeric_limits<double>::infinity();
            for(size_t i = 0 ; i < adv_nodes.size() ; i++){
                double move_eval = sa_minimax_search(adv_nodes[i], depth);

                if(move_eval > best_eval){
                    best_eval = move_eval;
                    best_move = adv_nodes[i];
                }
            }
            break;
        }
        case dtd::sa_node::Behavior::MIN:{
            double best_eval = std::numeric_limits<double>::infinity();
            for(size_t i = 0 ; i < adv_nodes.size() ; i++){
                double move_eval = sa_minimax_search(adv_nodes[i], depth);

                if(move_eval < best_eval){
                    best_eval = move_eval;
                    best_move = adv_nodes[i];
                }
            }
            break;
        }
        default:
            // no decision can be made from a luck node (they are not players)
            return nullptr;
            break;
    }

defer:
    for(auto ptr:adv_nodes)
        if(ptr != best_move) delete ptr;

    return best_move;
}

};

#endif // !STOCHASTIC_ADVERSARIAL_SEARCH_H
