#include <new>

#include "PseuWoW.h"
#include "tools.h"
#include <iostream>
#include <sys/timeb.h>



int main(int argc, char* argv[])
{
    std::cout << argc << " program arguments" << std::endl;
    for(int i = 0; i < argc; ++i) {
        std::cout << "Argument " << i << " " << argv[i] << std::endl;
    }

    struct timeb tp;
    ftime(&tp);
    setProcessStartTime((uint32)tp.time, (uint32)tp.millitm);

    PseuInstance i(NULL);
    i.Init();    
    return i.Run();
}
