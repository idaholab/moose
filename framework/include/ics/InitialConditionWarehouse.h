#ifndef INITIALCONDITIONWAREHOUSE_H
#define INITIALCONDITIONWAREHOUSE_H

#include <vector>

#include "InitialCondition.h"

/**
 * Typedef to hide implementation details
 */
typedef std::map<std::string, InitialCondition *>::iterator InitialConditionIterator;


class InitialConditionWarehouse
{
public:
  InitialConditionWarehouse();
  virtual ~InitialConditionWarehouse();

  InitialConditionIterator activeInitialConditionsBegin();
  InitialConditionIterator activeInitialConditionsEnd();

  InitialCondition * getInitialCondition(const std::string & var_name);

  void addIC(const std::string & var_name, InitialCondition *ic);

protected:
  std::map<std::string, InitialCondition *> _active_initial_conditions;
};

#endif // INITIALCONDITIONWAREHOUSE_H
