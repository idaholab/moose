/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "AddSlaveFluxVectorAction.h"
#include "Parser.h"
#include "FEProblem.h"

template<>
InputParameters validParams<AddSlaveFluxVectorAction>()
{
  return validParams<Action>();
}

AddSlaveFluxVectorAction::AddSlaveFluxVectorAction(const InputParameters & params) :
    Action(params)
{
}

void
AddSlaveFluxVectorAction::act()
{
  _problem->getNonlinearSystem().addVector("slave_flux", false, GHOSTED, true);
}


// DEPRECATED CONSTRUCTOR
AddSlaveFluxVectorAction::AddSlaveFluxVectorAction(const std::string & deprecated_name, InputParameters params) :
    Action(deprecated_name, params)
{
}
