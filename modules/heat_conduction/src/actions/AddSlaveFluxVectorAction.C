#include "AddSlaveFluxVectorAction.h"
#include "Parser.h"
#include "FEProblem.h"

template<>
InputParameters validParams<AddSlaveFluxVectorAction>()
{
  return validParams<Action>();
}

AddSlaveFluxVectorAction::AddSlaveFluxVectorAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AddSlaveFluxVectorAction::act()
{
  _problem->getNonlinearSystem().addVector("slave_flux", false, GHOSTED, true);
}
