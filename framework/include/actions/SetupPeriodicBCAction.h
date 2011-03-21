#ifndef SETUPPERIODICBCACTION_H
#define SETUPPERIODICBCACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class SetupPeriodicBCAction : public Action
{
public:
  SetupPeriodicBCAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<SetupPeriodicBCAction>();

#endif // SETUPPERIODICBCACTION_H
