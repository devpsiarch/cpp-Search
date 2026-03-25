#include <cmath>
#include <iostream>
#include <deque>
#include <unordered_map>
#include <vector>
#include <cmath>
#include "include/search.hpp"

class mazeNode : public dtd::state{
private:
    // the state the user choose to impliment
    int x;
    int y;

    // the user has to define a enum of a actions that can be done
    enum Actions {
        down,
        up,
        right,
        left,
        nothing
    };


    using TransitionModel = const std::unordered_map<Actions,std::pair<std::vector<int>,const char*>>;
    
    inline static TransitionModel transition_model{
        {Actions::down,  {{1, 0}  ,"down"}},
        {Actions::up,    {{-1, 0} ,"up"}},
        {Actions::left,  {{0, -1} ,"left"}},
        {Actions::right, {{0,  1} ,"right"}}
    };

    // the maze itself
    inline static const std::vector<std::vector<int>> map{
        {-2, 0 , 0 , 0 , 0, 0},
        {0 , 0 , 0 , 1 , 1, 0},
        {0 , 0 , 1 , 1 , 1, 0},
        {1 , 0 , 0 , 0 , 2, 0},
        {1 , 1 , 1 , 1 , 1, 1}
    };
    
    // and the goal position (optional depending on the criterion)
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

    // impliments the path traced during the search
    std::deque<Actions> traced_path;

    virtual bool is_goal() const noexcept override {
        return mazeNode::map[this->x][this->y] == 2;
    }

    mazeNode(int _x = 0,int _y = 0,int _path_cost = 0,const std::deque<Actions>& previous = {},Actions actions_performed = Actions::nothing) 
        : dtd::state(_path_cost) , x(_x) , y(_y) , traced_path(previous) {
        this->traced_path.push_front(actions_performed);
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
        for(const auto& pair:mazeNode::transition_model){
            dx = pair.second.first[0];
            dy = pair.second.first[1];
            if(is_valid(x+dx,y+dy) && !is_blocked(x+dx,y+dy)){
                dtd::state* new_state = new mazeNode(x+dx,dy+y,this->path_cost+1,this->traced_path,pair.first);
                ans.push_back(new_state);
            }
        }
        return ans;
    }

    virtual void trace_back_actions() noexcept override {
        std::cout << "Tracedback actions: \n";
        while(!this->traced_path.empty()){
            Actions act = this->traced_path.back();

            auto it = this->transition_model.find(act);
            if(it != this->transition_model.end()){
                std::cout << this->transition_model.at(act).second << ",";
            }else{
                // we reached the end "then we neet up the print" appended Actions::nothing 
            }

            this->traced_path.pop_back();
        }
        std::cout << "\nDone tracing back the actions.\n";
    }

};


int main(void){

    dtd::state* init = new mazeNode{};
    dtd::state* result = dtd::GraphSearchAlgorithm<dtd::Strategy::LIFO>(init);

    result->trace_back_actions();

    delete result;

    return 0;
}
