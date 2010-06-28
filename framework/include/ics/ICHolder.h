#ifndef ICHOLDER_H
#define ICHOLDER_H

#include <vector>

#include "InitialCondition.h"

/**
 * Typedef to hide implementation details
 */
typedef std::map<std::string, InitialCondition *>::iterator InitialConditionIterator;


class ICHolder
{
public:
  ICHolder(MooseSystem &sys);
  virtual ~ICHolder();

  InitialConditionIterator activeInitialConditionsBegin(THREAD_ID tid);
  InitialConditionIterator activeInitialConditionsEnd(THREAD_ID tid);

  InitialCondition * getInitialCondition(THREAD_ID tid, std::string var_name);

  void addIC(THREAD_ID tid, std::string var_name, InitialCondition *ic);

protected:
  std::vector<std::map<std::string, InitialCondition *> > _active_initial_conditions;

  MooseSystem &_moose_system;
};

#endif // BCHOLDER_H
