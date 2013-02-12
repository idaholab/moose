#include "AddSlaveFluxVectorAction.h"
#include "Parser.h"
#include "FEProblem.h"

static unsigned int n = 0;

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
  if (n == 0)
  {
    _problem->getNonlinearSystem().addVector("slave_flux", false, GHOSTED, true);
    ++n;
  }
}
