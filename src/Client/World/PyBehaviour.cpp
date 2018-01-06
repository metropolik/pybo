#include "PyBehaviour.h"

PyBehaviour::PyBehaviour()
{
    Py_Initialize();
}

void PyBehaviour::update()
{
//    PyRun_SimpleString("from time import time,ctime\n"
//                         "print('Today is',ctime(time()))\n");
}

PyBehaviour::~PyBehaviour()
{
    Py_Finalize();
}
