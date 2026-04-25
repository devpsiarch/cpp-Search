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
          "./include/search.hpp",
          "./include/local_search.hpp",
          "./include/genetic_search.hpp",
          "./include/adversarial_search.hpp",
          "./include/sa_search.hpp",
          "./include/csp.hpp"
        );
    OUROC_BUILD_CMD(&main,"g++",
                    "-Wall -Wextra",
                    "-fsanitize=address",
                    "-g",
                    FILE_TO_BUILD,"-o",SOLUTION_BIN
                    );

    // raylib visualization version
    OUROC(ray,SOLUTION_BIN,FILE_TO_BUILD,__FILE__,
          "include/search.hpp",
          "include/local_search.hpp",
          "genetic_search.hpp"
        );
    OUROC_BUILD_CMD(&ray,"g++",
                    "-Wall -Wextra",
                    "-fsanitize=address",
                    "-g",
                    "-I./raylib-5.5_linux_amd64/include/",
                    FILE_TO_BUILD,"-o",SOLUTION_BIN,
                    "-L./raylib-5.5_linux_amd64/lib/",
                    "-l:libraylib.a",
                    "-lm"
                    );


    if(!strcmp(argv[1],"main")) 
        ouroc_run_cmd(&main);
    
    else if(!strcmp(argv[1],"ray"))
        ouroc_run_cmd(&ray);

    else{
        printf("build.c :: you havnt defined said instruction.\n");
        exit(1);
    }


    OUROC_KILL(&main);
    OUROC_KILL(&ray);
    return 0;
}
