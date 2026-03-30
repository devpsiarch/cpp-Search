#include "ouroc.h"
#define OUROC_IMPLI

#define FILE_TO_BUILD "main.cpp"
#define SOLUTION_BIN "main"

int main(int argc,char** argv){

    if(argc != 2){
        printf("usage: ./build <object>\n");
        exit(1);
    }

    // normal version
    OUROC(main,SOLUTION_BIN,FILE_TO_BUILD,__FILE__,
          "include/search.hpp",
          "include/local_search.hpp"
        );
    OUROC_BUILD_CMD(&main,"g++",
                    "-Wall -Wextra",
                    "-fsanitize=address",
                    "-g",
                    FILE_TO_BUILD,"-o",SOLUTION_BIN
                    );


    OUROC(run,NULL,SOLUTION_BIN);
    OUROC_BUILD_CMD(&run,"./"SOLUTION_BIN);

    if(!strcmp(argv[1],"main")) 
        ouroc_run_cmd(&main);
    else if(!strcmp(argv[1],"run"))
        ouroc_run_cmd(&run);

    else if(!strcmp(argv[1],"all")) {
        ouroc_run_cmd(&main);
        ouroc_run_cmd(&run);
    }
    else{
        printf("build.c :: you havnt defined said instruction.\n");
        exit(1);
    }


    OUROC_KILL(&main);
    OUROC_KILL(&run);
    return 0;
}
