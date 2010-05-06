#include "ICHolder.h"

ICHolder::ICHolder(MooseSystem &sys)
  : _moose_system(sys)
{
  active_initial_conditions.resize(libMesh::n_threads());
}

ICHolder::~ICHolder()
{
}

InitialConditionIterator
ICHolder::activeInitialConditionsBegin(THREAD_ID tid)
{
  return active_initial_conditions[tid].begin();
}

InitialConditionIterator
ICHolder::activeInitialConditionsEnd(THREAD_ID tid)
{
  return active_initial_conditions[tid].end();
}


InitialCondition *
ICHolder::getInitialCondition(THREAD_ID tid, std::string var_name)
{
  InitialConditionIterator ic_iter = active_initial_conditions[tid].find(var_name);

  if (ic_iter == active_initial_conditions[tid].end())
    return NULL;

  return ic_iter->second;
}
