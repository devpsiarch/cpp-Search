#ifndef SEARCH_H
#define SEARCH_H

#include <queue>
#include <vector>
#include <iostream>
#include <deque>
#include <unordered_map>
#include <memory>

namespace dtd {

class state {
protected:
    // every state owns its own memory , if any problems arrise 
    // we will change this
 
    // we keep track of the parent state and the path cost 
    // the "action" and the "data that the search considers as state" should be implimented by the user
    state* parent_state;
    int path_cost;
    // enum::action <by the user>
    // ... state <by the user>


    // methodes used for explored set and comparing states
    // the deriving class must impliment them
    //
    virtual size_t generateHash() const = 0;
    virtual bool isEqualTo(const dtd::state* other) const = 0;
    virtual bool isLessThen(const dtd::state* other) const = 0;

public: 

    // the user has to define the expand function that expands the 
    // the current state to many other states , the user has to define the logic 
    // of the program himself
    virtual std::vector<state*> expand() = 0;

    // to see the expanded array
    static void print_states(std::vector<state*>& other){
        for(unsigned int i = 0 ; i < other.size() ; i++){
            other[i]->print_state();
        }
    }

    // a function that checks if we are in the goal state or not
    virtual bool isGoal() const noexcept = 0;

    friend struct PolymorphicHasher;
    friend struct PolymorphicEqual;
    friend struct PolymorphicCmp;

    // when we want to create a state for a particual problem , 
    // we inherite from the base class and define the needed constructors 
    //
    // the default constructor should behave as the same as a initial state 

    state(state* _parent_state,int _path_cost) 
        : parent_state(_parent_state),
        path_cost(_path_cost){
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
        std::cout << "state:: (path_cost) = (" << this->path_cost << ")\n";
    }
};

struct PolymorphicHasher {
    size_t operator()(const dtd::state* b) const {
        return b ? b->generateHash() : 0;
    }
};

struct PolymorphicEqual {
    bool operator()(const dtd::state* lhs, const dtd::state* rhs) const {
        if (lhs == rhs) return true;
        if (!lhs || !rhs) return false;
        return lhs->isEqualTo(rhs);
    }
};

struct PolymorphicCmp {
    bool operator()(const dtd::state* lhs, const dtd::state* rhs) const {
        if (lhs == rhs) return true;
        if (!lhs || !rhs) return false;
        return lhs->isLessThen(rhs);
    }
};

enum Strategy {FIFO,LIFO,PRIORITY};

template <Strategy strat = Strategy::FIFO,typename cmp = std::less<int>>
class frontier {
private:
    std::deque<std::unique_ptr<dtd::state>> data;
    std::unordered_map<dtd::state*, bool,dtd::PolymorphicHasher,dtd::PolymorphicEqual> seen;
public:
    dtd::state* get(){
        if(data.empty()) return nullptr;
        dtd::state* ans = nullptr;
        switch (strat) {
            case dtd::Strategy::FIFO:
                ans = this->data.front().release();
                this->data.pop_front();
                break;
            case dtd::Strategy::LIFO:
                ans = this->data.back().release();
                this->data.pop_back();
                break;
            default:
                break;
        }
        seen[ans] = false;
        return ans;
    }
    void add(dtd::state* next_state){
        seen[next_state] = true;
        this->data.emplace_back(std::move(next_state));
    }
    bool empty() const noexcept {return this->data.empty();}

};

template <typename cmp>
class frontier<Strategy::PRIORITY,cmp> {
private:
    std::priority_queue<dtd::state*,std::vector<dtd::state*>,cmp> data;
    std::unordered_map<dtd::state*, bool,dtd::PolymorphicHasher,dtd::PolymorphicEqual> seen;
public:
    dtd::state* get(){
        if(data.empty()) return nullptr;

        dtd::state* current = this->data.top();

        this->data.pop();

        seen[current] = false;
        return current;
    }
    void add(dtd::state* next_state){
        seen[next_state] = true;
        this->data.push(std::move(next_state));
    }
    bool empty() const noexcept {return this->data.empty();}

    ~frontier(){
        while(!this->empty()){
            dtd::state* killed = this->get();
            delete killed;
        }
    }

};


};
#endif // !SEARCH_H
