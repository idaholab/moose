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

  // MultiAppTransfers by default will execute with their associated MultiApp. These flags will be added by FEProblem when the transfer is added.
  MultiMooseEnum multi_transfer_execute_on(params.get<MultiMooseEnum>("execute_on").getRawNames() + " same_as_multiapp", "same_as_multiapp");
  params.set<MultiMooseEnum>("execute_on") = multi_transfer_execute_on;

  return params;
}

// Free function to clear special execute_on option before initializing SetupInterface
InputParameters & removeSpecialOption(InputParameters & params)
{
  params.set<MultiMooseEnum>("execute_on").erase("SAME_AS_MULTIAPP");
  return params;
}

MultiAppTransfer::MultiAppTransfer(const std::string & name, InputParameters parameters) :
    /**
     * Here we need to remove the special option that indicates to the user that this object will follow it's associated
     * Multiapp execute_on. This non-standard option is not understood by SetupInterface. In the absence of any execute_on
     * parameters, FEProblem will populate the execute_on MultiMooseEnum with the values from the associated MultiApp.
     */
    Transfer(name, removeSpecialOption(parameters)),
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
