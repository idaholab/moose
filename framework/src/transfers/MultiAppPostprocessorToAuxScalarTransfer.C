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
registerMooseObject("MooseApp", MultiAppPostprocessorToAuxScalarTransfer);

InputParameters
MultiAppPostprocessorToAuxScalarTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription("Transfers from a postprocessor to a scalar auxiliary variable.");
  params.addRequiredParam<PostprocessorName>(
      "from_postprocessor", "The name of the Postprocessor to transfer the value from.");
  params.addRequiredParam<VariableName>(
      "to_aux_scalar", "The name of the scalar AuxVariable to transfer the value to.");
  return params;
}

MultiAppPostprocessorToAuxScalarTransfer::MultiAppPostprocessorToAuxScalarTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _from_pp_name(getParam<PostprocessorName>("from_postprocessor")),
    _to_aux_name(getParam<VariableName>("to_aux_scalar"))
{
  if (_directions.size() != 1)
    paramError("direction", "This transfer is only unidirectional");
}

void
MultiAppPostprocessorToAuxScalarTransfer::execute()
{
  TIME_SECTION("MultiAppPostprocessorToAuxScalarTransfer::execute()",
               5,
               "Performing transfer between a scalar variable and a postprocessor");

  // Perform action based on the transfer direction
  switch (_current_direction)
  {
    // MultiApp -> MultiApp
    case BETWEEN_MULTIAPP:
    {
      for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
      {
        if (getFromMultiApp()->hasLocalApp(i))
        {
          // Extract the postprocessor that is being transferred
          FEProblemBase & from_problem = getFromMultiApp()->appProblemBase(i);
          Real pp_value = from_problem.getPostprocessorValueByName(_from_pp_name);

          if (getToMultiApp()->hasLocalApp(i))
          {
            // Get reference to the AuxVariable where the postprocessor will be passed
            MooseVariableScalar & scalar =
                getToMultiApp()->appProblemBase(i).getScalarVariable(_tid, _to_aux_name);

            scalar.reinit();

            // Set all values of the AuxVariable to the value of the postprocessor
            scalar.setValues(pp_value);

            // Update the solution
            scalar.insert(scalar.sys().solution());
            scalar.sys().solution().close();
          }
        }
      }
      break;
    }

    // main app -> MultiApp
    case TO_MULTIAPP:
    {
      // Extract the postprocessor that is being transferred
      FEProblemBase & from_problem = getToMultiApp()->problemBase();
      Real pp_value = from_problem.getPostprocessorValueByName(_from_pp_name);

      // Loop through each of the sub apps
      for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
        if (getToMultiApp()->hasLocalApp(i))
        {
          // Get reference to the AuxVariable where the postprocessor will be passed
          MooseVariableScalar & scalar =
              getToMultiApp()->appProblemBase(i).getScalarVariable(_tid, _to_aux_name);

          scalar.reinit();

          // Set all values of the AuxVariable to the value of the postprocessor
          scalar.setValues(pp_value);

          // Update the solution
          scalar.insert(scalar.sys().solution());
          scalar.sys().solution().close();
        }
      break;
    }

    // MultiApp -> main app
    case FROM_MULTIAPP:
    {
      // The number of sub applications
      unsigned int num_apps = getFromMultiApp()->numGlobalApps();

      // The AuxVariable for storing the postprocessor values from the sub app
      MooseVariableScalar & scalar =
          getFromMultiApp()->problemBase().getScalarVariable(_tid, _to_aux_name);

      // Ensure that the variable is up to date
      scalar.reinit();

      // The dof indices for the scalar variable of interest
      auto && dof = scalar.dofIndices();

      // Error if there is a size mismatch between the scalar AuxVariable and the number of sub apps
      if (num_apps != scalar.sln().size())
        mooseError("The number of sub apps (",
                   num_apps,
                   ") must be equal to the order of the scalar AuxVariable (",
                   scalar.order(),
                   ")");

      // Loop over each sub-app and populate the AuxVariable values from the postprocessors
      for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
        if (getFromMultiApp()->hasLocalApp(i) && getFromMultiApp()->isRootProcessor())
          // Note: This can't be done using MooseScalarVariable::insert() because different
          // processors will be setting dofs separately.
          scalar.sys().solution().set(
              dof[i],
              getFromMultiApp()->appProblemBase(i).getPostprocessorValueByName(_from_pp_name));

      scalar.sys().solution().close();

      break;
    }
  }
}
