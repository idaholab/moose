#ifndef ADDPERIODICBCACTION_H
#define ADDPERIODICBCACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class AddPeriodicBCAction : public Action
{
public:
  AddPeriodicBCAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddPeriodicBCAction>();

#endif // ADDPERIODICBCACTION_H
