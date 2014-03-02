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


  // Perform action based on the transfer direction
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
        if (_multi_app->hasLocalApp(i))
        {
          // Get reference to the AuxVariable where the postprocessor will be passed
          MooseVariableScalar & scalar =  _multi_app->appProblem(i)->getScalarVariable(_tid, _to_aux_name);

          // Set all values of the AuxVariable to the value of the postprocessor
          scalar.sln().setAllValues(pp_value);

          // Update the solution
          scalar.insert(scalar.sys().solution());
        }
      break;
    }

    // SubApp -> MasterApp
    case FROM_MULTIAPP:
    {
      // The number of sub applications
      unsigned int num_apps = _multi_app->numGlobalApps();

      // The AuxVariable for storing the postprocessor values from the sub app
      MooseVariableScalar & scalar =  _multi_app->problem()->getScalarVariable(_tid, _to_aux_name);

      // The dof indices for the scalar variable of interest
      std::vector<dof_id_type> & dof = scalar.dofIndices();

      // Error if there is a size mismatch between the scalar AuxVariable and the number of sub apps
      if (num_apps != scalar.sln().size())
        mooseError("The number of sub apps (" << num_apps << ") must be equal to the order of the scalar AuxVariable (" << scalar.order() << ")");

      // Loop over each sub-app and populate the AuxVariable values from the postprocessors
      for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
        if (_multi_app->hasLocalApp(i))
          scalar.sys().solution().set(dof[i], _multi_app->appProblem(i)->getPostprocessorValue(_from_pp_name));

      break;
    }
  }
}
