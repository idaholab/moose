#ifndef ADDAUXVARIABLEACTION_H
#define ADDAUXVARIABLEACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class AddAuxVariableAction : public Action
{
public:
  AddAuxVariableAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddAuxVariableAction>();

#endif // ADDAUXVARIABLEACTION_H
