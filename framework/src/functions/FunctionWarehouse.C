#include "FunctionWarehouse.h"

FunctionWarehouse::FunctionWarehouse(MooseSystem &sys):
  _functions(libMesh::n_threads())
{
}

FunctionWarehouse::~FunctionWarehouse()
{
  std::vector<std::map<std::string, Function *> >::iterator i;
  for (i = _functions.begin(); i != _functions.end(); ++i)
    for (FunctionIterator j = i->begin(); j != i->end(); ++j)
      delete j->second;
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
FunctionWarehouse::addFunction(THREAD_ID tid, std::string fname, Function * func)
{
  _functions[tid][fname] = func;
}
