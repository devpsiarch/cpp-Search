#include <iostream>
#include <vector>

#include "include/search.hpp"

class mazeNode : public dtd::state{
private:
    // the state the user choose to impliment
    int x;
    int y;
    inline static const std::vector<std::vector<int>> directions{{1,0},{-1,0},{0,1},{0,-1}};
    inline static const std::vector<std::vector<int>> map{
        {-2, 0 , 0 , 0 , 0, 0},
        {0 , 0 , 0 , 1 , 1, 0},
        {0 , 0 , 1 , 1 , 1, 0},
        {1 , 0 , 0 , 0 , 2, 0},
        {1 , 1 , 1 , 1 , 1, 1}
    };
    
    inline static const std::vector<int> goal_state{4,3};

    bool is_valid(int nx,int ny)const noexcept{
        return ((nx >= 0 && nx < map.size())&&(ny >= 0 && ny < map[0].size()));
    }

    bool is_blocked(int nx,int ny) const noexcept {
        return map[nx][ny] == 1;
    }

protected:
    virtual size_t generateHash() const override final{
        return std::hash<int>{}(this->x) + std::hash<int>{}(this->y);
    }
    virtual bool isEqualTo(const dtd::state* other) const override final{
        const mazeNode* rhs = dynamic_cast<const mazeNode*>(other);
        if(!rhs) return false;
        if(rhs->x != this->x || rhs->y != this->y) return false;
        return true;
    }
    virtual bool isLessThen(const dtd::state* other) const override final {
        const mazeNode* rhs = dynamic_cast<const mazeNode*>(other);
        if(!rhs) return false;
        if(rhs->x <= this->x || rhs->y <= this->y) return false;
        return true;       
    }

public:

    virtual bool isGoal() const noexcept override {
        return this->x == mazeNode::goal_state[0] && this->y == mazeNode::goal_state[1];
    }

    mazeNode(int _x = 0,int _y = 0,state* _parent_state = nullptr,int _path_cost = 0) 
        : dtd::state(_parent_state,_path_cost) , 
            x(_x) , y(_y) {
    }
    ~mazeNode() override {
        // do any stuff you want here
    }

    void print_state() const noexcept override {
        std::cout << "mazeNode:: (x,y,path_cost) = (" << this->x << "," << this->y << "," << this->path_cost << ")\n";
    }

    std::vector<dtd::state*> expand() final override {
        std::vector<dtd::state*> ans;
        int dx,dy;
        for(const auto& dir:mazeNode::directions){
            dx = dir[0];
            dy = dir[1];
            if(is_valid(x+dx,y+dy) && !is_blocked(x+dx,y+dy)){
                dtd::state* new_state = new mazeNode(x+dx,dy+y,this,this->path_cost+1);
                ans.push_back(new_state);
            }
        }
        return ans;
    }

};


int main(void){

    dtd::frontier<dtd::Strategy::PRIORITY,dtd::PolymorphicCmp> test;

    mazeNode* someNode = new mazeNode{}; // (0,0)
    
    std::vector<dtd::state*> expand0 = someNode->expand();
    dtd::state::print_states(expand0);

    for(auto*ptr:expand0)
        test.add(ptr);

    dtd::state* gotten = test.get();
    gotten->print_state();

    delete gotten;

    delete someNode;
    return 0;
}
