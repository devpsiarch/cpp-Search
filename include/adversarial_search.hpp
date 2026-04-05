#ifndef  ADVERSARIAL_SEARCH_H
#define ADVERSARIAL_SEARCH_H

#include <limits>
#include <vector>

namespace dtd {

class adversarial_node {
    // we leave the implimentation of the state and data member to the deriving class
    // except the current Behavior which is hardcoded
private:
 
    // we also leave the trasition model to the deriving class

public:

    enum class Behavior {
        MIN,MAX
    };

    Behavior behavior;

    virtual double evaluate() const noexcept = 0;
    virtual bool terminal_test() const noexcept = 0;
    virtual std::vector<dtd::adversarial_node*> generate_successors() const noexcept = 0;

    virtual void print() const noexcept = 0;

    virtual ~adversarial_node() = default;
};



double minimax_search(dtd::adversarial_node* init,unsigned int depth){
    if(depth == 0 || init->terminal_test()) return init->evaluate();

    std::vector<dtd::adversarial_node*> adv_nodes = init->generate_successors();

    if(adv_nodes.size() == 0)
        return init->evaluate();

    double node_eval;

    switch(init->behavior){
        case dtd::adversarial_node::Behavior::MAX:{
            double max_eval = -std::numeric_limits<double>::infinity();
            for(size_t i = 0 ; i < adv_nodes.size() ; i++){
                double eval = minimax_search(adv_nodes[i],depth-1);
                max_eval = std::max(max_eval,eval);
            }
            node_eval = max_eval;
        }
        case dtd::adversarial_node::Behavior::MIN:{
            double min_eval = std::numeric_limits<double>::infinity();
            for(size_t i = 0 ; i < adv_nodes.size() ; i++){
                double eval = minimax_search(adv_nodes[i],depth-1);
                min_eval = std::min(min_eval,eval);
            }
            node_eval = min_eval;
        }
    }

defer:
    for(auto ptr:adv_nodes) delete ptr;
    return node_eval;
}

dtd::adversarial_node* minimax_decision(dtd::adversarial_node* init,unsigned int depth = 3){
    dtd::adversarial_node* best_move = nullptr;
    
    std::vector<dtd::adversarial_node*> adv_nodes = init->generate_successors();

    if(adv_nodes.size() == 0)
        return best_move;
    
    double best_eval = -std::numeric_limits<double>::infinity();

    for(size_t i = 0 ; i < adv_nodes.size(); i++){
        double move_eval = minimax_search(adv_nodes[i], depth);

        if(move_eval > best_eval){
            best_eval = move_eval;
            best_move = adv_nodes[i];
        }
    }

defer:
    for(auto ptr:adv_nodes)
        if(ptr != best_move) delete ptr;

    return best_move;
}


double alpha_beta_search(
    dtd::adversarial_node* init,
    unsigned int depth,
    double alpha,
    double beta
){
    if(depth == 0 || init->terminal_test()) return init->evaluate();

    std::vector<dtd::adversarial_node*> adv_nodes = init->generate_successors();

    if(adv_nodes.size() == 0)
        return init->evaluate();

    double node_eval;

    switch(init->behavior){
        case dtd::adversarial_node::Behavior::MAX:{
            double max_eval = -std::numeric_limits<double>::infinity();
            for(size_t i = 0 ; i < adv_nodes.size() ; i++){
                double eval = alpha_beta_search(adv_nodes[i],depth-1,alpha,beta);
                alpha = std::max(eval,alpha);
                max_eval = std::max(max_eval,eval);

                if(alpha >= beta) break;
            }
            node_eval = max_eval;
            break;
        }
        case dtd::adversarial_node::Behavior::MIN:{
            double min_eval = std::numeric_limits<double>::infinity();
            for(size_t i = 0 ; i < adv_nodes.size() ; i++){
                double eval = alpha_beta_search(adv_nodes[i],depth-1,alpha,beta);
                beta = std::min(eval,beta);
                min_eval = std::min(min_eval,eval);

                if(alpha >= beta) break;

            }
            node_eval = min_eval;
            break;
        }
    }

defer:
    for(auto ptr:adv_nodes) delete ptr;
    return node_eval;
}

dtd::adversarial_node* alpha_beta_decision(dtd::adversarial_node* init,unsigned int depth = 3){
    dtd::adversarial_node* best_move = nullptr;
    
    std::vector<dtd::adversarial_node*> adv_nodes = init->generate_successors();

    if(adv_nodes.size() == 0)
        return best_move;
    
    double best_eval = -std::numeric_limits<double>::infinity();

    for(size_t i = 0 ; i < adv_nodes.size(); i++){
        double move_eval = alpha_beta_search(
            adv_nodes[i],depth,-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity()
        );

        if(move_eval > best_eval){
            best_eval = move_eval;
            best_move = adv_nodes[i];
        }
    }

defer:
    for(auto ptr:adv_nodes)
        if(ptr != best_move) delete ptr;
    return best_move;
}




};

#endif
