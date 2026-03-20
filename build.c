#include "ouroc.h"
#define OUROC_IMPLI

#define FILE_TO_BUILD "main.cpp"
#define SOLUTION_BIN "main"

int main(int argc,char** argv){
    OUROC(main,SOLUTION_BIN,FILE_TO_BUILD,__FILE__,"include/search.hpp");
    OUROC_BUILD_CMD(&main,"g++ -Wall -Wextra -fsanitize=address",FILE_TO_BUILD,"-o",SOLUTION_BIN);

    OUROC(run,NULL,SOLUTION_BIN);
    OUROC_BUILD_CMD(&run,"./"SOLUTION_BIN);


    ouroc_run_cmd(&main);
    ouroc_run_cmd(&run);

    OUROC_KILL(&main);
    OUROC_KILL(&run);
    return 0;
}
