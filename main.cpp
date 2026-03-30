#include <iostream>
#include <random>
#include <string>
#include "include/local_search.hpp"

class Nqueens : public dtd::local_node {
public:
    // each rank is the row in the square board
    // the integer expresses where in that row a queen is
    std::string ranks;
    const size_t board_size;

    void populate_diag(std::vector<std::vector<int>>& buffer,const int r,const int c) const noexcept {
        for(int i = r+1, j = c+1 ; i < buffer.size() && j < buffer.size() ; i++ , j++){
            buffer[i][j] += 1;
        }
        for(int i = r-1, j = c+1 ; i >= 0 && j < buffer.size() ; i-- , j++){
            buffer[i][j] += 1;
        }
        for(int i = r-1, j = c-1 ; i >= 0 && j >= 0 ; i-- , j--){
            buffer[i][j] += 1;
        }
        for(int i = r+1, j = c-1 ; i < buffer.size() && j >= 0 ; i++ , j--){
            buffer[i][j] += 1;
        }
    }
    void populate_hori(std::vector<std::vector<int>>& buffer,const int r,const int c) const noexcept {
        for(int i = 0 ; i < buffer.size() ;i++){
            if(i == r) continue;
            buffer[i][c] += 1; 
        }
    }
    void populate_vert(std::vector<std::vector<int>>& buffer,const int r,const int c) const noexcept {
        for(int i = 0 ; i < buffer.size() ; i++){
            if(i == c) continue;
            buffer[r][i] += 1;
        } 
    }

    Nqueens(size_t N) : board_size(N) , ranks(N,'0'){}
    Nqueens(const std::string& ref) : board_size(ref.length()) , ranks(ref) {}
    ~Nqueens() = default;

    virtual void print() const noexcept override {
        std::cout << "state: " << this->ranks << " cost: " << this->objective_function() << '\n';
    }


    static dtd::local_node* get_radom_from_space(size_t board_size) noexcept{
        static std::random_device _rnd{};
        static std::mt19937 _engine{_rnd()};
        std::uniform_int_distribution<int> _norm{0,(int)board_size-1};

        Nqueens* rnd = new Nqueens(board_size);
        for(unsigned int i = 0; i < board_size ; i++){
            rnd->ranks[i] = '0' + _norm(_engine);
        }

        return rnd;
    }

    // returns the number of attacked queens
    virtual int objective_function() const noexcept override {
        std::vector<std::vector<int>> board_buffer(board_size, std::vector<int>(board_size, 0));
        // we draw the buffer attack map
        for(int i = 0 ; i < ranks.length() ; i++){
            int file = ranks[i] - '0';
            populate_diag(board_buffer,i,file);
            populate_hori(board_buffer,i,file);
            populate_vert(board_buffer,i,file);
            board_buffer[i][file] += 1;
        }
        // return the number of attacking queen for every queen
        int cost = 0;
        for(int i = 0; i < ranks.size() ; i++){
            cost += board_buffer[i][ranks[i]-'0'];
        }
        return cost - board_size; // because we count a queen attacking itself
    }
    virtual std::vector<dtd::local_node*> get_successors() const noexcept override {
        std::string cpy = this->ranks; // copy 
        std::vector<dtd::local_node*> children;
        for(int i = 0 ; i < this->ranks.length() ; i++){
            if(cpy[i] < ('0'+board_size-1)){
                cpy[i]++;
                children.push_back(new Nqueens(cpy));
                cpy[i]--;
            }
            if(cpy[i] > '0'){
                cpy[i]--;
                children.push_back(new Nqueens(cpy));
                cpy[i]++;
            }
        }
        return children;
    }
};


int main(void){

    dtd::local_node* init = Nqueens::get_radom_from_space(4);
 
    dtd::local_node* res = dtd::stochastic_hill_climbing_algorithm(init,[&](const std::vector<dtd::local_node*>& ref){
        std::vector<float> distro(ref.size(),0.f);
        std::vector<float> costs(ref.size());
        float mx = 0.f;
        for(int i = 0 ; i < ref.size() ; i++){
            costs[i] = ref[i]->objective_function();
            mx = std::max<float>(mx,costs[i]);
        }
        for(int i = 0; i < distro.size() ; i++){
            distro[i] = (mx - costs[i]) + 1;
        }
        return distro;
    });


    // dtd::local_node* res = dtd::stochastic_beam_search<dtd::MinimizeObjectiveFunction,3>(init,[&](const std::vector<dtd::local_node*>& ref){
    //     std::vector<float> distro(ref.size(),0.f);
    //     std::vector<float> costs(ref.size());
    //     float mx = 0.f;
    //     for(int i = 0 ; i < ref.size() ; i++){
    //         costs[i] = ref[i]->objective_function();
    //         mx = std::max<float>(mx,costs[i]);
    //     }
    //     for(int i = 0; i < distro.size() ; i++){
    //         distro[i] = (mx - costs[i]) + 1; // we shift all by one to avoid div by 0 error
    //     }
    //     return distro;
    // });

    // dtd::local_node* res = dtd::beam_search<dtd::MinimizeObjectiveFunction,3>(init);

    // dtd::local_node* res = dtd::simulated_annealing<dtd::exponential_cooling>(init, 1000.0f,0.95f);

    // dtd::local_node* res = dtd::first_choice_hill_climbing(init,[&](){
    //     return Nqueens::get_radom_from_space(4);
    // });

    // std::vector<dtd::local_node*> start(5);
    //
    // for(size_t i = 0 ; i < 5 ; i++){
    //     start[i] = Nqueens::get_radom_from_space(4);
    // }
    //
    // dtd::local_node* res = dtd::random_restart_hill_climbing(start);



    //
    // dtd::local_node* res = dtd::hill_climbing_algorithm(init);
 

    if(!res) return 1;

    std::cout << "Gotten best: ";
    res->print();


    delete res;

    return 0;
}
