#include "AddStabilizerAction.h"
#include "Factory.h"
#include "Parser.h"
#include "MProblem.h"

template<>
InputParameters validParams<AddStabilizerAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.addRequiredParam<std::string>("variable", "The name of the variable this Stabilizer will act on.");
  return params;
}

AddStabilizerAction::AddStabilizerAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddStabilizerAction::act() 
{
  _parser_handle._problem->addStabilizer(_type, getShortName(), _moose_object_pars);
}
