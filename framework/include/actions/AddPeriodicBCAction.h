#ifndef ADDPERIODICBCACTION_H
#define ADDPERIODICBCACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "MooseObjectAction.h"

#include <string>

class AddPeriodicBCAction : public MooseObjectAction
{
public:
  AddPeriodicBCAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddPeriodicBCAction>();

#endif // ADDPERIODICBCACTION_H
