#include <cmath>
#include <iostream>
#include <vector>
#include <cmath>
#include "include/search.hpp"

#define DOWN {1,0}
#define UP {-1,0}
#define RIGHT {0,1}
#define LEFT {0,-1}

class mazeNode : public dtd::state{
private:
    // the state the user choose to impliment
    int x;
    int y;

    inline static const std::vector<std::vector<int>> directions{DOWN,RIGHT,UP,LEFT};
    inline static const std::vector<std::vector<int>> map{
        {-2, 0 , 0 , 0 , 0, 0},
        {0 , 0 , 0 , 1 , 1, 0},
        {0 , 0 , 1 , 1 , 1, 0},
        {1 , 0 , 0 , 0 , 2, 0},
        {1 , 1 , 1 , 1 , 1, 1}
    };
    
    inline static const std::vector<int> goal_position{3,4};

    bool is_valid(int nx,int ny)const noexcept{
        if(nx >= (int)map.size() || nx < 0) return false;
        if(ny >= (int)map[0].size() || ny < 0) return false;
        return true;
    }

    bool is_blocked(int nx,int ny) const noexcept {
        return map[nx][ny] == 1;
    }

protected:
    virtual size_t generateHash() const override final{
        return std::hash<int>{}(this->x) + std::hash<int>{}(this->y);
    }
    virtual bool isEqualTo(const dtd::state* other) const override final{
        std::cout << other << " " << this << '\n';
        if(other == nullptr) return false;
        const mazeNode* rhs = dynamic_cast<const mazeNode*>(other);
        std::cout << "after rhs casting: " << rhs << '\n';
        if(!rhs) return false;
        if(rhs->x != this->x || rhs->y != this->y) return false;
        return true;
    }
    virtual bool isLessThen(const dtd::state* other) const override final {
        if(!other) return false;
        const mazeNode* rhs = dynamic_cast<const mazeNode*>(other);
        if(!rhs) return false;
        // SWO
        if (this->x != rhs->x) {
            return this->x < rhs->x;
        }
        return this->y < rhs->y;
    }


    // for example we can override the Heuristic methode and define it
    virtual int Heuristic() const noexcept override {
        const float dx = (this->x - mazeNode::goal_position[0]);
        const float dy = (this->y - mazeNode::goal_position[1]);

        return static_cast<int>(std::sqrt(dx*dx+dy*dy));
    }

        
public:

    virtual bool is_goal() const noexcept override {
        return mazeNode::map[this->x][this->y] == 2;
    }

    mazeNode(int _x = 0,int _y = 0,int _path_cost = 0) 
        : dtd::state(_path_cost) , 
            x(_x) , y(_y) {
    }
    ~mazeNode() override {
        // do any stuff you want here (our states for this class are not on the heap)
    }

    virtual void print_state() const noexcept override {
        std::cout << "mazeNode := (x,y,g,h,f) = (" 
            << this->x << "," 
            << this->y << "," 
            << this->path_cost << ","
            << this->Heuristic() << ","
            << this->evaluator()
        << ")\n";
    }

    std::vector<dtd::state*> expand() final override {
        std::vector<dtd::state*> ans;
        int dx,dy;
        for(const auto& dir:mazeNode::directions){
            dx = dir[0];
            dy = dir[1];
            if(is_valid(x+dx,y+dy) && !is_blocked(x+dx,y+dy)){
                dtd::state* new_state = new mazeNode(x+dx,dy+y,this->path_cost+1);
                ans.push_back(new_state);
            }
        }
        return ans;
    }

};


int main(void){

    dtd::state* init = new mazeNode{};
    dtd::state* result = dtd::TreeSearchAlgorithm<dtd::Strategy::PRIORITY>(init);

    delete result;

    return 0;

    dtd::frontier<dtd::Strategy::PRIORITY,dtd::PolymorphicLessThen> test;

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
