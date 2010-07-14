#include "FunctionWarehouse.h"
#include "MooseSystem.h"

FunctionWarehouse::FunctionWarehouse(MooseSystem &sys):
  _moose_system(sys),
  _functions(libMesh::n_threads())
{
}

FunctionWarehouse::~FunctionWarehouse()
{
  //TODO delete Functions created in parsing system?
}

FunctionIterator
FunctionWarehouse::activeFunctionsBegin(THREAD_ID tid)
{
  return _functions[tid].begin();
}

FunctionIterator
FunctionWarehouse::activeFunctionsEnd(THREAD_ID tid)
{
  return _functions[tid].end();
}

Function &
FunctionWarehouse::getFunction(THREAD_ID tid, std::string fname)
{
  FunctionIterator iter = _functions[tid].find(fname);

  if (iter == _functions[tid].end())
    mooseError("No Function by name: " + fname);

  return *(iter->second);
}

void
FunctionWarehouse::addFunction(THREAD_ID tid, std::string fname, Function *func)
{
  std::cout << "Adding Function to Warehouse: " << fname << "\n";
  _functions[tid][fname] = func;
}
