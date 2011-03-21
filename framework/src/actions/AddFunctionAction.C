#include "AddFunctionAction.h"
#include "Factory.h"
#include "Moose.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddFunctionAction>()
{
  return validParams<MooseObjectAction>();
}

AddFunctionAction::AddFunctionAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddFunctionAction::act() 
{
#ifdef DEBUG
  std::cerr << "Acting on AddFunctionAction\n";
  std::cerr << "\tFunction: " << getShortName() << "\n";
#endif

  _parser_handle._problem->addFunction(_type, getShortName(), _moose_object_pars);
}
