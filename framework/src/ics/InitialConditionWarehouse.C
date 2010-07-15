#include "InitialConditionWarehouse.h"

InitialConditionWarehouse::InitialConditionWarehouse(MooseSystem &sys)
  : _moose_system(sys)
{
  _active_initial_conditions.resize(libMesh::n_threads());
}

InitialConditionWarehouse::~InitialConditionWarehouse()
{
}

InitialConditionIterator
InitialConditionWarehouse::activeInitialConditionsBegin(THREAD_ID tid)
{
  return _active_initial_conditions[tid].begin();
}

InitialConditionIterator
InitialConditionWarehouse::activeInitialConditionsEnd(THREAD_ID tid)
{
  return _active_initial_conditions[tid].end();
}


InitialCondition *
InitialConditionWarehouse::getInitialCondition(THREAD_ID tid, std::string var_name)
{
  InitialConditionIterator ic_iter = _active_initial_conditions[tid].find(var_name);

  if (ic_iter == _active_initial_conditions[tid].end())
    return NULL;

  return ic_iter->second;
}

void
InitialConditionWarehouse::addIC(THREAD_ID tid, std::string var_name, InitialCondition *ic)
{
  _active_initial_conditions[tid][var_name] = ic;
}
