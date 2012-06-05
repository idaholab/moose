/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "CopyNodalVarsAction.h"
#include "Parser.h"
#include "FEProblem.h"
#include "ActionWarehouse.h"

#include <map>

template<>
InputParameters validParams<CopyNodalVarsAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<int>("initial_from_file_timestep", 2, "Gives the timestep for which to read a solution from a file for a given variable");
  params.addParam<std::string>("initial_from_file_var", "Gives the name of a variable for which to read an initial condition from a mesh file");

  return params;
}

CopyNodalVarsAction::CopyNodalVarsAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
CopyNodalVarsAction::act()
{

  if (isParamValid("initial_from_file_var"))
  {
    SystemBase * system;
    // Is this a NonlinearSystem variable or an AuxiliarySystem variable?
    if (getAction() == "copy_nodal_vars")
      system = &_problem->getNonlinearSystem();
    else
      system = &_problem->getAuxiliarySystem();

    system->addVariableToCopy(getParam<std::string>("initial_from_file_var"),
                              getParam<int>("initial_from_file_timestep"));
  }
}
