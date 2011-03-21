#include "AddDamperAction.h"
#include "Factory.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddDamperAction>()
{
   return validParams<MooseObjectAction>();
}

AddDamperAction::AddDamperAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddDamperAction::act() 
{
  _parser_handle._problem->addDamper(_type, getShortName(), _moose_object_pars);
}
