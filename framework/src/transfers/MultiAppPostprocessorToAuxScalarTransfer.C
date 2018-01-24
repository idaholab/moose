//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppPostprocessorToAuxScalarTransfer.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseTypes.h"
#include "MooseVariableScalar.h"
#include "MultiApp.h"
#include "SystemBase.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

// Define the input parameters
template <>
InputParameters
validParams<MultiAppPostprocessorToAuxScalarTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<PostprocessorName>(
      "from_postprocessor",
      "The name of the Postprocessor in the Master to transfer the value from.");
  params.addRequiredParam<VariableName>(
      "to_aux_scalar",
      "The name of the scalar Aux variable in the MultiApp to transfer the value to.");
  return params;
}

MultiAppPostprocessorToAuxScalarTransfer::MultiAppPostprocessorToAuxScalarTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _from_pp_name(getParam<PostprocessorName>("from_postprocessor")),
    _to_aux_name(getParam<VariableName>("to_aux_scalar"))
{
}

void
MultiAppPostprocessorToAuxScalarTransfer::execute()
{
  _console << "Beginning PostprocessorToAuxScalarTransfer " << name() << std::endl;

  // Perform action based on the transfer direction
  switch (_direction)
  {
    // MasterApp -> SubApp
    case TO_MULTIAPP:
    {
      // Extract the postprocessor that is being transferd
      FEProblemBase & from_problem = _multi_app->problemBase();
      Real pp_value = from_problem.getPostprocessorValue(_from_pp_name);

      // Loop through each of the sub apps
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
        if (_multi_app->hasLocalApp(i))
        {
          // Get reference to the AuxVariable where the postprocessor will be passed
          MooseVariableScalar & scalar =
              _multi_app->appProblemBase(i).getScalarVariable(_tid, _to_aux_name);

          scalar.reinit();

          // Set all values of the AuxVariable to the value of the postprocessor
          scalar.setValues(pp_value);

          // Update the solution
          scalar.insert(scalar.sys().solution());
          scalar.sys().solution().close();
        }
      break;
    }

    // SubApp -> MasterApp
    case FROM_MULTIAPP:
    {
      // The number of sub applications
      unsigned int num_apps = _multi_app->numGlobalApps();

      // The AuxVariable for storing the postprocessor values from the sub app
      MooseVariableScalar & scalar =
          _multi_app->problemBase().getScalarVariable(_tid, _to_aux_name);

      // Ensure that the variable is up to date
      scalar.reinit();

      // The dof indices for the scalar variable of interest
      std::vector<dof_id_type> & dof = scalar.dofIndices();

      // Error if there is a size mismatch between the scalar AuxVariable and the number of sub apps
      if (num_apps != scalar.sln().size())
        mooseError("The number of sub apps (",
                   num_apps,
                   ") must be equal to the order of the scalar AuxVariable (",
                   scalar.order(),
                   ")");

      // Loop over each sub-app and populate the AuxVariable values from the postprocessors
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
        if (_multi_app->hasLocalApp(i) && _multi_app->isRootProcessor())
          // Note: This can't be done using MooseScalarVariable::insert() because different
          // processors will be setting dofs separately.
          scalar.sys().solution().set(
              dof[i], _multi_app->appProblemBase(i).getPostprocessorValue(_from_pp_name));

      scalar.sys().solution().close();

      break;
    }
  }

  _console << "Finished PostprocessorToAuxScalarTransfer " << name() << std::endl;
}
