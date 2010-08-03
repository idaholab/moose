#include "InitialConditionWarehouse.h"

InitialConditionWarehouse::InitialConditionWarehouse()
{
}

InitialConditionWarehouse::~InitialConditionWarehouse()
{
}

InitialConditionIterator
InitialConditionWarehouse::activeInitialConditionsBegin()
{
  return _active_initial_conditions.begin();
}

InitialConditionIterator
InitialConditionWarehouse::activeInitialConditionsEnd()
{
  return _active_initial_conditions.end();
}


InitialCondition *
InitialConditionWarehouse::getInitialCondition(const std::string & var_name)
{
  InitialConditionIterator ic_iter = _active_initial_conditions.find(var_name);

  if (ic_iter == _active_initial_conditions.end())
    return NULL;

  return ic_iter->second;
}

void
InitialConditionWarehouse::addIC(const std::string & var_name, InitialCondition *ic)
{
  _active_initial_conditions[var_name] = ic;
}
