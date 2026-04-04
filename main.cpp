#include <chrono>
#include <cmath>
#include <thread>
#include <vector>
#include "include/search.hpp"
#include <unordered_map>
#include <raylib.h>

class mazeNode : public dtd::state{
public:
    // the state the user choose to impliment
    int x;
    int y;

private:
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

public:
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


struct visulizer {
    int width;
    int height;

    int cell_size;

    enum cell_type {
        NOT,
        EXPLORED,
        CURRENT,
        IN_FRONTIER,
        GOAL,
        BLOCK,
    };

    std::vector<std::vector<cell_type>> cell_buffer;

    visulizer(int rows,int cols,int cell_,int fps,const char* name) 
        : width(rows*cell_) ,
        height(cols*cell_),
        cell_size(cell_) ,
        cell_buffer(width/cell_size,std::vector<cell_type>(height/cell_size,cell_type::NOT)){
        InitWindow(width, height, name);
        SetTargetFPS(fps);
    }
    ~visulizer(){
        CloseWindow();
    }



};


enum cell_type {
    NOT = 0,
    BLOCK = 1,
    GOAL = 2,
    EXPLORED,
    CURRENT,
    IN_FRONTIER,
};



// Initialize an empty buffer with the correct dimensions
static std::vector<std::vector<cell_type>> cell_buffer(
    mazeNode::map.size(), 
    std::vector<cell_type>(mazeNode::map[0].size())
);


static constexpr int cell_size = 50;

void render_cell_buffer() noexcept {
    for(unsigned int i = 0 ; i < cell_buffer.size() ; i++){
        for(unsigned int j = 0 ; j < cell_buffer[0].size() ; j++){
            Color selected_color;
            switch (cell_buffer[i][j]) {
                case cell_type::NOT:
                    selected_color = GRAY;
                    break;
                case cell_type::EXPLORED:
                    selected_color = GREEN;
                    break;
                case cell_type::CURRENT:
                    selected_color = BLUE;
                    break;
                case cell_type::IN_FRONTIER:
                    selected_color = RED;
                    break;
                case cell_type::BLOCK:
                    selected_color = BLACK;
                    break;
                case cell_type::GOAL:
                    selected_color = YELLOW;
                    break;
            }
            DrawRectangle(i*cell_size, j*cell_size, cell_size-2, cell_size-2,selected_color);
        }
    }
}

int main(void){
    for(size_t i = 0; i < mazeNode::map.size(); ++i) {
        for (size_t j = 0; j < mazeNode::map[i].size(); ++j) {
            cell_buffer[i][j] = static_cast<cell_type>(mazeNode::map[i][j]);
        }
    }

    InitWindow(cell_size*cell_buffer.size(), cell_size*cell_buffer[0].size(),"Simple maze");
    SetTargetFPS(60);
    
    while(!WindowShouldClose()){


        dtd::state* init = new mazeNode{};
        auto res = dtd::GUIGraphSearchAlgorithm<dtd::Strategy::FIFO>(init, [&](const auto& front,const auto& expl,dtd::state* current_s){
            for(const dtd::state*node:front.get_data()){
                if(!node) exit(1);
                auto ptr = dynamic_cast<const mazeNode*>(node);
                if(!ptr) exit(1);

                cell_buffer[ptr->x][ptr->y] = cell_type::IN_FRONTIER;
            }
            for(const dtd::state*node:expl.get_data()){
                if(!node) exit(1);
                auto ptr = dynamic_cast<const mazeNode*>(node);
                if(!ptr) exit(1);

                cell_buffer[ptr->x][ptr->y] = cell_type::EXPLORED;
            }
            if(!current_s) exit(1);
            auto ptr = dynamic_cast<const mazeNode*>(current_s);
            if(!ptr) exit(1);       

            cell_buffer[ptr->x][ptr->y] = cell_type::CURRENT;

            BeginDrawing();
                render_cell_buffer();
            EndDrawing();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            if(WindowShouldClose()) exit(0);
        });

        delete res;
        break;
    }


    return 0;
}
