#ifndef INTEGRITYCHECKACTION_H
#define INTEGRITYCHECKACTION_H

#include "R7Action.h"

class IntegrityCheckAction;

template<>
InputParameters validParams<IntegrityCheckAction>();

/**
 * Check the integrity of the simulation
 */
class IntegrityCheckAction : public R7Action
{
public:
  IntegrityCheckAction(const std::string & name, InputParameters parameters);
  virtual ~IntegrityCheckAction();

  virtual void act();

protected:
};



#endif /* INTEGRITYCHECKACTION_H */
