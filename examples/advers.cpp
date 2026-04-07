#include "../include/adversarial_search.hpp"
#include <iostream>

class tictactoe : public dtd::adversarial_node {
public:
    enum Word {
        X = 0,
        O = 1,
        NONE = 2
    };

    std::vector<Word> board;
    Word player;

    tictactoe() : board(9,Word::NONE) , player(Word::X) {
        if(player == Word::X) behavior = dtd::adversarial_node::Behavior::MAX;
        else behavior = dtd::adversarial_node::Behavior::MIN;
    }


    void make_play(int r,int c,Word op) {
        board[r*3+c] = op;
        player = op == Word::X ? Word::O : Word::X;
    }

    // how far away are we from winning - how far are we from loosing
    virtual double evaluate() const noexcept override {
        // Checking for Rows for X or O victory.
        for (int row = 0; row < 3; row++)
        {
            if (board[row * 3 + 0] == board[row * 3 + 1] && board[row * 3 + 1] == board[row * 3 + 2])
            {
                if (board[row * 3 + 0] == Word::X)
                   return +10;
                else if (board[row * 3 + 0] == Word::O)
                   return -10;
            }
        }

        // Checking for Columns for X or O victory.
        for (int col = 0; col < 3; col++)
        {
            if (board[0 * 3 + col] == board[1 * 3 + col] && board[1 * 3 + col] == board[2 * 3 + col])
            {
                if (board[0 * 3 + col] == Word::X)
                    return +10;
                else if (board[0 * 3 + col] == Word::O)
                    return -10;
            }
        }

        // Checking for Diagonals for X or O victory.
        // Main diagonal (top-left to bottom-right): indices 0, 4, 8
        if (board[0] == board[4] && board[4] == board[8])
        {
            if (board[0] == Word::X)
                return +10;
            else if (board[0] == Word::O)
                return -10;
        }
        
        // Anti-diagonal (top-right to bottom-left): indices 2, 4, 6
        if (board[2] == board[4] && board[4] == board[6])
        {
            if (board[2] == Word::X)
                return +10;
            else if (board[2] == Word::O)
                return -10;
        }
        // Else if none of them have won then return 0
        return 0;
    }
    virtual bool terminal_test() const noexcept override {
        if(evaluate() == 0) return false;
        return true;
    }
    virtual std::vector<dtd::adversarial_node*> generate_successors() const noexcept override {
        std::vector<dtd::adversarial_node*> next_states;

        for(int i = 0 ;  i < 3 ; i ++){
            for(int j = 0 ; j < 3 ; j++){
                if(board[i*3+j] == Word::NONE){
                    tictactoe* next = new tictactoe(*this);
                    next->make_play(i,j,this->player);
                    next_states.push_back(next);
                }
            }
        }

        return next_states;
    }

    virtual void print() const noexcept override {
         for(int i = 0 ;  i < 3 ; i ++){
            for(int j = 0 ; j < 3 ; j++){
                switch (board[i*3+j]){
                    case Word::X:
                        std::cout << "X";
                        break;
                    case Word::O:
                        std::cout << "O";
                        break;
                    case Word::NONE:
                        std::cout << " ";
                        break;
                }
                std::cout << '|';
            }
            std::cout << "\n";
        }       
    }
};

int main(){

    dtd::adversarial_node* init = new tictactoe();

    

    auto res = dtd::minimax_decision(init);

    if(!res) exit(1);

    while(!init->terminal_test()){
        auto next = dtd::minimax_decision(init,10);

        delete init;
        init = next;

        if(init->terminal_test()){
            std::cout << "AI won!\n";
            break;
        }

        init->print();

        int r,c;
        std::cin >> r >> c;

        auto ptr = dynamic_cast<tictactoe*>(init);

        ptr->make_play(r,c,ptr->player);


        if(init->terminal_test()){
            std::cout << "Human won!\n";
            break;
        }


    }

    init->print();

    delete init;
    delete res;
    return 0;
}
