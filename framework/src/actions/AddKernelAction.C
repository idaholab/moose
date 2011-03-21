#include "AddKernelAction.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddKernelAction>()
{
  return validParams<MooseObjectAction>();
}

AddKernelAction::AddKernelAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddKernelAction::act()
{
  is_kernels_action = Parser::pathContains(_name, "Kernels");

  if (is_kernels_action)
    _parser_handle._problem->addKernel(_type, getShortName(), _moose_object_pars);
  else
    _parser_handle._problem->addAuxKernel(_type, getShortName(), _moose_object_pars);
}
