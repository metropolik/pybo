#include <new>

#include "PseuWoW.h"
#include <iostream>



int main(int argc, char* argv[])
{
    std::cout << argc << " program arguments" << std::endl;
    for(int i = 0; i < argc; ++i) {
        std::cout << "Argument " << i << " " << argv[i] << std::endl;
    }
    PseuInstance i(NULL);
    i.Init();    
    return i.Run();
}
