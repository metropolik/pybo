#include "PyBehaviour.h"

PyBehaviour::PyBehaviour()
{
    Py_Initialize();
}

void PyBehaviour::update()
{
    
}

PyBehaviour::~PyBehaviour()
{
    Py_Finalize();
}
