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

#include "MultiAppTransfer.h"

#include "Transfer.h"
#include "MooseTypes.h"
#include "FEProblem.h"

template<>
InputParameters validParams<MultiAppTransfer>()
{
  InputParameters params = validParams<Transfer>();
  params.addRequiredParam<MultiAppName>("multi_app", "The name of the MultiApp to use.");

  params.addRequiredParam<MooseEnum>("direction", MultiAppTransfer::directions(), "Whether this Transfer will be 'to' or 'from' a MultiApp.");

  return params;
}

MultiAppTransfer::MultiAppTransfer(const std::string & name, InputParameters parameters) :
    Transfer(name, parameters),
    _multi_app(_fe_problem.getMultiApp(getParam<MultiAppName>("multi_app"))),
    _direction(getParam<MooseEnum>("direction"))
{
}

void
MultiAppTransfer::variableIntegrityCheck(const AuxVariableName & var_name) const
{
  for (unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
    if (_multi_app->hasLocalApp(i) && !find_sys(_multi_app->appProblem(i)->es(), var_name))
      mooseError("Cannot find variable " << var_name << " for " << _name << " Transfer");
}
