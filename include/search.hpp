#ifndef SEARCH_H
#define SEARCH_H

#include <queue>
#include <set>
#include <vector>
#include <iostream>
#include <deque>

namespace dtd {

class state {
protected:
    // every state owns its own memory , if any problems arrise 
    // we will change this
 
    // we keep track of the parent state and the path cost 
    // the "action" and the "data that the search considers as state" should be implimented by the user
    int path_cost;
    // enum::action <by the user>
    // ... state <by the user>


    // methodes used for explored set and comparing states
    // the deriving class must impliment them
    //
    virtual size_t generateHash() const = 0;
    virtual bool isEqualTo(const dtd::state* other) const = 0;
    virtual bool isLessThen(const dtd::state* other) const = 0;

    // heuristics and path expansion priority for the case of "informed search"

    virtual int Heuristic() const noexcept {return 0;} // by default no heuristic , override for other
    virtual int PathCost() const noexcept {return this->path_cost;} // by default cummulative path cost , override for other

    // you cannot override this , this is how we define the evaluator
    virtual int evaluator() const noexcept final {return this->Heuristic() + this->PathCost();} 

public: 

    // the user has to define the expand function that expands the 
    // the current state to many other states , the user has to define the logic 
    // of the program himself
    virtual std::vector<state*> expand() = 0;

    // BETA function , traces back the actions taken by the agent
    virtual void trace_back_actions() noexcept = 0;

    // to see the expanded array
    static void print_states(std::vector<state*>& other){
        for(unsigned int i = 0 ; i < other.size() ; i++){
            other[i]->print_state();
        }
    }

    // a function that checks if we are in the goal state or not
    virtual bool is_goal() const noexcept = 0;

    friend struct PolymorphicHasher;
    friend struct PolymorphicEqual;
    friend struct PolymorphicLessThen;
    friend struct PolymorphicOrderer;

    // when we want to create a state for a particual problem , 
    // we inherite from the base class and define the needed constructors 
    //
    // the default constructor should behave as the same as a initial state 

    state(int _path_cost) 
        : path_cost(_path_cost){
    }
    virtual ~state(){
        // should do nothing 
    }


    // print state function 
    // the should impliment any kind of printing for debugging purposes 
    // and to check out the logic of programs 
    //
    // we provide one for less shyst to worry about
    virtual void print_state(void) const noexcept{
        std::cout << "dtd::state := (x,y,g,h,f) = (" 
            << this->path_cost << ","
            << this->Heuristic() << ","
            << this->evaluator()
        << ")\n";
    }
};

struct PolymorphicHasher {
    size_t operator()(const dtd::state* b) const {
        return b ? b->generateHash() : 0;
    }
};

struct PolymorphicEqual {
    bool operator()(const dtd::state* lhs, const dtd::state* rhs) const {
        if (!lhs || !rhs) return false;
        if (lhs == rhs) return true;
        return lhs->isEqualTo(rhs);
    }
};

struct PolymorphicLessThen {
    bool operator()(const dtd::state* lhs, const dtd::state* rhs) const {
        if (!lhs && !rhs) return false;
        if(!lhs) return true;
        if(!rhs) return false;
        if (lhs == rhs) return false;
        return lhs->isLessThen(rhs);
    }
};

struct PolymorphicOrderer {
    bool operator()(const dtd::state* lhs, const dtd::state* rhs) const {
        if (!lhs && !rhs) return false;
        if(!lhs) return true;
        if(!rhs) return false;
        if (lhs == rhs) return false;
        return lhs->evaluator() > rhs->evaluator();
    }
};

enum Strategy {FIFO,LIFO,PRIORITY};

template <Strategy strat = Strategy::FIFO,typename cmp = dtd::PolymorphicOrderer>
class frontier {
private:
    std::deque<dtd::state*> data;
    std::set<dtd::state*,dtd::PolymorphicLessThen> seen;
public:
    dtd::state* get(){
        if(data.empty()) return nullptr;
        dtd::state* ans = nullptr;
        switch (strat) {
            case dtd::Strategy::FIFO:
                ans = this->data.front();
                this->data.pop_front();
                break;
            case dtd::Strategy::LIFO:
                ans = this->data.back();
                this->data.pop_back();
                break;
            default:
                break;
        }
        if(ans)
            seen.erase(ans);
        return ans;
    }
    void add(dtd::state* next_state){
        seen.insert(next_state);
        this->data.push_back(next_state);
    }
    bool empty() const noexcept {return this->data.empty();}
    bool contains(dtd::state* other) noexcept {
        return this->seen.find(other) != this->seen.end();
    }
    ~frontier(){
        while(!this->empty()){
            dtd::state* killed = this->get();
            delete killed;
        }
    }
};

template <typename cmp>
class frontier<Strategy::PRIORITY,cmp> {
private:
    std::priority_queue<dtd::state*,std::vector<dtd::state*>,cmp> data;
    std::set<dtd::state*,PolymorphicLessThen> seen;
public:
    dtd::state* get(){
        if(data.empty()) return nullptr;

        dtd::state* current = this->data.top();

        this->data.pop();

        if(current)
            seen.erase(current);
        return current;
    }
    void add(dtd::state* next_state){
        seen.insert(next_state);
        this->data.push(next_state);
    }
    bool empty() const noexcept {return this->data.empty();}
    bool contains(dtd::state* other) noexcept {return this->seen.find(other) != this->seen.end();}

    ~frontier(){
        while(!this->empty()){
            dtd::state* killed = this->get();
            delete killed;
        }
    }

};



template <Strategy strat = Strategy::FIFO>
dtd::state* TreeSearchAlgorithm(dtd::state* initial_state,int limit = 10000){
    frontier<strat,dtd::PolymorphicLessThen> front;
    front.add(initial_state);

            unsigned int move_counter = 0;

    dtd::state* ans = nullptr;

    while(!front.empty() && limit > 0){

        dtd::state* current_state = front.get();
 
        if(!current_state) std::cout << "[ERROR]: gotten nullptr from frontier\n";

        if(current_state->is_goal()){
            std::cout << "GOAL FOUND in [" << move_counter << "] moves.\n";
            ans = current_state;
            goto defer;
        }

        std::vector<dtd::state*> expanded_nodes = current_state->expand();

        std::cout << "We expanded these now:\n";
        dtd::state::print_states(expanded_nodes);
        std::cout << "result is done.\n";

        delete current_state;

        for(unsigned int i = 0 ; i < expanded_nodes.size() ; i++){
            if(expanded_nodes[i] && !front.contains(expanded_nodes[i])) 
                front.add(expanded_nodes[i]);
            else{
                delete expanded_nodes[i];
            }
        }
        
        move_counter++;

        limit--;
    }

defer:
    return ans;
}

template <Strategy strat = Strategy::FIFO>
dtd::state* GraphSearchAlgorithm(dtd::state* initial_state,int limit = 10000){
    frontier<strat,PolymorphicLessThen> front;
    front.add(initial_state);

            unsigned int move_counter = 0;

    dtd::state* ans = nullptr;

    frontier<strat,PolymorphicLessThen> explored_set;

    while(!front.empty() && limit > 0){

        dtd::state* current_state = front.get();
 
        // some validation for debugging
        if(!current_state) std::cout << "[ERROR]: gotten nullptr from frontier\n";
        if(current_state->is_goal()){
            std::cout << "GOAL FOUND in [" << move_counter << "] moves.\n";
            ans = current_state;
            goto defer;
        }

        // add to explored_set 
        explored_set.add(current_state);

        std::vector<dtd::state*> expanded_nodes = current_state->expand();

        std::cout << "We expanded these now:\n";
        dtd::state::print_states(expanded_nodes);
        std::cout << "result is done.\n";

        // we dont delete the current node visited , since we insert it to a frontier 
        // it will be clearned in the end
        //
        // any other allocated node is either in the explored_set frontier (will be clearned) 
        // or in the expanded_nodes array each iteration (will be killed during the search)
        // or in the frontier which also will be cleaned

        for(unsigned int i = 0 ; i < expanded_nodes.size() ; i++){
            if(expanded_nodes[i] && !front.contains(expanded_nodes[i]) && !explored_set.contains(expanded_nodes[i])) 
                front.add(expanded_nodes[i]);
            else{
                delete expanded_nodes[i];
            }
        }
        
        move_counter++;

        limit--;
    }

defer:
    return ans;
}

};
#endif // !SEARCH_H
