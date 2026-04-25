#include "./include/sa_search.hpp"
#include <iostream>

class test_node : public dtd::sa_node {
public:

    enum action {
        a1,a2,
        b1,b2,
        nada
    };

    inline static const int takes = 3;

    int current_money;
    int current_turns;

    action act_taken;

    test_node(int cm,int ct,Behavior b,action act = action::nada,Behavior p = Behavior::LUCK) 
        : dtd::sa_node(b) , current_money(cm) , current_turns(ct) , act_taken(act){}
    
    virtual double evaluate() const noexcept override {return current_money;}
    virtual bool terminal_test() const noexcept override {
        // win
        if(current_turns == takes && current_money > 0) return true;
        // lost
        return current_money == 0;
    }
    
    virtual std::vector<std::pair<dtd::sa_node*,double>> expand_luck_outcome() const noexcept override {
        if(this->behavior != Behavior::LUCK){
            std::cerr << "trying to luck expand non luck nodes , invalid.\n";
            exit(0);
        }
        std::vector<std::pair<dtd::sa_node*,double>> res;
        
        Behavior next_behavior;
        if(this->act_taken == action::a1 || this->act_taken == action::a2) next_behavior = Behavior::MIN;
        else next_behavior = Behavior::MAX;

        switch (this->act_taken) {
            case action::a1:{
                std::pair<dtd::sa_node*,double> p1 = {new test_node(current_money+2,current_turns+1,next_behavior),0.75f};
                std::pair<dtd::sa_node*,double> p2 = {new test_node(current_money-3,current_turns+1,next_behavior),0.25f};
                res.emplace_back(p1);
                res.emplace_back(p2);
                break;
            }
            case action::a2:{
                std::pair<dtd::sa_node*,double> p1 = {new test_node(current_money+8,current_turns+1,next_behavior),0.125f};
                std::pair<dtd::sa_node*,double> p2 = {new test_node(current_money-6,current_turns+1,next_behavior),0.875};
                res.emplace_back(p1);
                res.emplace_back(p2);
                break;
            }
            case action::b1:{
                std::pair<dtd::sa_node*,double> p1 = {new test_node(current_money+1,current_turns+1,next_behavior),0.333f};
                std::pair<dtd::sa_node*,double> p2 = {new test_node(current_money-6,current_turns+1,next_behavior),0.667};
                res.emplace_back(p1);
                res.emplace_back(p2);
                break;
            }
            case action::b2:{
                std::pair<dtd::sa_node*,double> p1 = {new test_node(current_money,current_turns+1,next_behavior),1.0f};
                res.emplace_back(p1);
                break;
            }
            default:
                std::cerr << "trying to luck expand non luck nodes (nada) , invalid.\n";
                exit(0);
                break;
        }
        return res;
    }
    virtual std::vector<dtd::sa_node*> generate_successors() const noexcept override {
        std::vector<sa_node*> ans;
        switch (this->behavior) {
            case Behavior::MAX:{
                ans.push_back(new test_node(current_money,current_turns,dtd::sa_node::Behavior::LUCK,action::a1));
                ans.push_back(new test_node(current_money,current_turns,dtd::sa_node::Behavior::LUCK,action::a2));
                break;
            }
            case Behavior::MIN:{
                ans.push_back(new test_node(current_money,current_turns,dtd::sa_node::Behavior::LUCK,action::b1));
                ans.push_back(new test_node(current_money,current_turns,dtd::sa_node::Behavior::LUCK,action::b2));
                break;
            }
            default:
                std::cerr << "trying to expand luck nodes ... invalid , should use expand_luck_outcome.\n";
                exit(0);
                break;
        }
        return ans;
    }

    void print() const noexcept override {
        // 1. Convert the inherited Behavior enum to a string
        std::string type_str = (behavior == MAX) ? "MAX" : (behavior == MIN ? "MIN" : "LUCK");

        // 2. Print all the data
        std::cout << "[Behavior: " << type_str 
                  << " | Turns Left: " << current_turns 
                  << " | Money: $" << current_money 
                  << " | Last Action: " << action_to_string(act_taken) 
                  << "]\n";
    }
private:
std::string action_to_string(action a) const noexcept {
        switch (a) {
            case a1:   return "a1";
            case a2:   return "a2";
            case b1:   return "b1";
            case b2:   return "b2";
            case nada: return "nada";
            default:   return "unknown";
        }
    }
};

int main() {

    dtd::sa_node* init = new test_node(3,0,dtd::sa_node::Behavior::MAX);
    
    init->print();

    auto res = dtd::sa_minimax_decision(init, 10);
    auto res_v = dtd::sa_minimax_search(init, 10);


    std::cout << "--------------------\n"; 

    res->print();
    std::cout << "best eval is: "<< res_v << '\n';


    delete init;
    delete res;

defer:
    return 0;
}
