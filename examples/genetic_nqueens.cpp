#include <algorithm>
#include <iostream>
#include <random>
#include <string>
#include "../include/genetic_search.hpp"
#include "../include/local_search.hpp"

class Nqueens : public dtd::genetic_state {
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


    static dtd::genetic_state* get_radom_from_space(size_t board_size) noexcept{
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

    virtual void mutate(float probability = 0.5f) noexcept override {
        static std::random_device _rnd{};
        static std::mt19937 _engine{_rnd()};
        std::uniform_real_distribution<float> _norm{0.0f,1.0f};
        std::uniform_int_distribution<size_t> _dnorm;

        // i choose this weird mutation behavior , experimentation and messing around 
        // may lead to better empirical results
        if(_norm(_engine) < probability){
            for(size_t i = 0 ; i < this->ranks.size() ; i++){
                int current_val = this->ranks[i] - '0';
                int next_val = current_val + _dnorm(_engine);
                
                this->ranks[i] = '0' + (next_val % board_size);
                this->ranks[i] = '0' + (((next_val % board_size) + board_size) % board_size);
            }
        }
    }
    virtual genetic_state* cross_over(genetic_state* other) const noexcept override {
        static std::random_device _rnd{};
        static std::mt19937 _engine{_rnd()};
        std::uniform_int_distribution<size_t> _dnorm{0,this->ranks.size()};

        auto son = new Nqueens(this->ranks.size());
    
        size_t cut_index = _dnorm(_engine);
        auto father = dynamic_cast<Nqueens*>(other);

        if(!father){
            std::cout << "[UNEXPECTED]: GIVEN NON VALID POINTER TYPE IN CROSS OVER.\n";
            exit(1);
        }

        for(size_t i = 0 ; i < cut_index ; i++){
            son->ranks[i] = father->ranks[i];
        }
        for(size_t i = cut_index ; i < this->ranks.size() ; i++){
            son->ranks[i] = this->ranks[i];
        }

        return son;
    }
};

int main(void){

    std::vector<dtd::genetic_state*> start(5);

    for(size_t i = 0 ; i < 5 ; i++){
        start[i] = Nqueens::get_radom_from_space(4);
    }

    dtd::local_node* res = dtd::simple_genetic_algorithm(
                     start,
                     0.5f,
                     [&](const std::vector<dtd::genetic_state*>& ref){
                        std::vector<int> distro(ref.size(),0.f);
                        std::vector<int> costs(ref.size());
                        int mx = 0;
                        for(int i = 0 ; i < ref.size() ; i++){
                            costs[i] = ref[i]->objective_function();
                            mx = std::max(mx,costs[i]);
                        }
                        for(int i = 0; i < distro.size() ; i++){
                            distro[i] = (mx - costs[i]) + 1; // we shift all by one to avoid div by 0 error
                        }
                        return distro;
                     },
                     [&](const std::vector<dtd::genetic_state*>& pop){
                        return pop[0];
                     });

    res->print();

    return 0;
}
