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

#include "MultiAppPostprocessorToAuxScalarTransfer.h"

// Moose
#include "MooseTypes.h"
#include "FEProblem.h"

// libMesh
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

// Define the input parameters
template<>
InputParameters validParams<MultiAppPostprocessorToAuxScalarTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<PostprocessorName>("from_postprocessor", "The name of the Postprocessor in the Master to transfer the value from.");
  params.addRequiredParam<VariableName>("to_aux_scalar", "The name of the scalar Aux variable in the MultiApp to transfer the value to.");
  return params;
}

MultiAppPostprocessorToAuxScalarTransfer::MultiAppPostprocessorToAuxScalarTransfer(const std::string & name, InputParameters parameters) :
    MultiAppTransfer(name, parameters),
    _from_pp_name(getParam<PostprocessorName>("from_postprocessor")),
    _to_aux_name(getParam<VariableName>("to_aux_scalar"))
{
}

void
MultiAppPostprocessorToAuxScalarTransfer::execute()
{
  switch(_direction)
  {
    // MasterApp -> SubApp
    case TO_MULTIAPP:
    {

      // Extract the postprocessor that is being transferd
      FEProblem & from_problem = *_multi_app->problem();
      Real pp_value = from_problem.getPostprocessorValue(_from_pp_name);

      // Loop through each of the sub apps
      for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
        if(_multi_app->hasLocalApp(i))
        {
          // Get reference to the AuxVariable where the postprocessor will be passed
          MooseVariableScalar & var =  _multi_app->appProblem(i)->getScalarVariable(_tid, _to_aux_name);

          // Set all values of the AuxVariable to the value of the postprocessor
          var.sln().setAllValues(pp_value);

          // Update the solution
          var.insert(var.sys().solution());
        }
      break;
    }

    // SubApp -> MasterApp
    //TODO: This should be possible if the AuxVariable has a component for each subapp (i.e., order = num. of sub apps), but this
    // can't be implemente until #2331 is complete.
    case FROM_MULTIAPP:
    {
      mooseError("Not yet implemented.");
      break;
    }
  }
}
