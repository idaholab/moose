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

#include "AddICAction.h"
#include "FEProblem.h"
#include "MooseTypes.h"
#include "MooseUtils.h"

template<>
InputParameters validParams<AddICAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}


AddICAction::AddICAction(const std::string & name, InputParameters params) :
    MooseObjectAction(name, params)
{
}

void
AddICAction::act()
{
  std::vector<std::string> elements;
  MooseUtils::tokenize(_name, elements);

  // The variable name will be the second to last element in the path name
  std::string & var_name = elements[elements.size()-2];
  _moose_object_pars.set<VariableName>("variable") = var_name;
  _problem->addInitialCondition(_type, getShortName(), _moose_object_pars);
}
