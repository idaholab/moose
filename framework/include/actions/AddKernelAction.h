#ifndef ADDKERNELACTION_H
#define ADDKERNELACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "MooseObjectAction.h"

#include <string>

class AddKernelAction : public MooseObjectAction
{
public:
  AddKernelAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  bool is_kernels_action;
};

template<>
InputParameters validParams<AddKernelAction>();

#endif // ADDKERNELACTION_H
