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
   std::cerr << "Constructing AddDiracKernelAction"
            << "\nname: " << _name
             << "\naction: " << _action << "\n\n";
}

void
AddDiracKernelAction::act()
{
  std::cerr << "Acting on AddDiracKernelAction"
            << "\nname: " << _name
            << "\naction: " << _action << "\n\n";

  _parser_handle._problem->addDiracKernel(_type, getShortName(), _moose_object_pars);
}
