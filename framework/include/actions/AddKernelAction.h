#ifndef ADDKERNELACTION_H
#define ADDKERNELACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class AddKernelAction : public Action
{
public:
  AddKernelAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddKernelAction>();

#endif // ADDKERNELACTION_H
