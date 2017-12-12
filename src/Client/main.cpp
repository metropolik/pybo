#include <new>

#include "PseuWoW.h"



int main(int argc, char* argv[])
{
    try
    {              
        PseuInstance i(NULL);
        i.SetConfDir("./conf/");
        i.SetScpDir("./scripts/");
        i.Init();
        i.Run();
        return 0;
	}
    catch (...)
    {
        printf("ERROR: Unhandled exception in main thread!\n");
        return 1;
    }
}
