#ifndef ADDVARIABLEACTION_H
#define ADDVARIABLEACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class AddVariableAction : public Action
{
public:
  AddVariableAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddVariableAction>();

#endif // ADDVARIABLEACTION_H
