#include "AddDiracKernelAction.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddDiracKernelAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddDiracKernelAction::AddDiracKernelAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddDiracKernelAction::act()
{
  _parser_handle._problem->addDiracKernel(_type, getShortName(), _moose_object_pars);
}
