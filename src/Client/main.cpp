#include <new>

#include "PseuWoW.h"



int main(int argc, char* argv[])
{
    try
    {
        std::cout << "Starting..." << std::endl;
        // 1 instance is enough for now
        PseuInstance *_i;
        _i = new PseuInstance(NULL);
        _i->SetConfDir("./conf/");
        _i->SetScpDir("./scripts/");

        if(_i->Init())
        {
            _i->Run();
        }
        else
        {
            getchar(); // if init failed, wait for keypress before exit
        }
        delete _i;
        return 0;
	}
    catch (...)
    {
        printf("ERROR: Unhandled exception in main thread!\n");
        return 1;
    }
}
