#ifndef ADDDIRACKERNELACTION_H
#define ADDDIRACKERNELACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "MooseObjectAction.h"

#include <string>

class AddDiracKernelAction : public MooseObjectAction
{
public:
  AddDiracKernelAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  bool is_kernels_action;
};

template<>
InputParameters validParams<AddDiracKernelAction>();

#endif // ADDDIRACKERNELACTION_H
