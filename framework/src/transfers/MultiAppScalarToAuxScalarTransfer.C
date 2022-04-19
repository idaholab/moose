//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppScalarToAuxScalarTransfer.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseTypes.h"
#include "MooseVariableScalar.h"
#include "MultiApp.h"
#include "SystemBase.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

// Define the input parameters
registerMooseObject("MooseApp", MultiAppScalarToAuxScalarTransfer);

InputParameters
MultiAppScalarToAuxScalarTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription("Transfers data from a scalar variable to an auxiliary scalar "
                             "variable from different applications.");
  params.addRequiredParam<VariableName>("source_variable",
                                        "The name of the scalar variable to "
                                        "transfer the value from.");
  params.addRequiredParam<VariableName>("to_aux_scalar",
                                        "The name of the scalar auxiliary variable to "
                                        "transfer the value to.");
  return params;
}

MultiAppScalarToAuxScalarTransfer::MultiAppScalarToAuxScalarTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _from_variable_name(getParam<VariableName>("source_variable")),
    _to_aux_name(getParam<VariableName>("to_aux_scalar"))
{
  if (_directions.size() != 1)
    paramError("direction", "This transfer is only unidirectional");
}

void
MultiAppScalarToAuxScalarTransfer::execute()
{
  TIME_SECTION(
      "MultiAppScalarToAuxScalarTransfer::execute()", 5, "Performing a scalar variable transfer");

  // Perform action based on the transfer direction
  switch (_current_direction)
  {
    // main app to multi_app
    case TO_MULTIAPP:
    {
      // Extract the scalar variable that is being transferred
      FEProblemBase & from_problem = getToMultiApp()->problemBase();
      MooseVariableScalar * from_variable =
          &from_problem.getScalarVariable(_tid, _from_variable_name);

      // Loop through each of the sub apps
      for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
        if (getToMultiApp()->hasLocalApp(i))
        {
          // Get reference to the scalar variable that will be written
          MooseVariableScalar * to_variable =
              &getToMultiApp()->appProblemBase(i).getScalarVariable(_tid, _to_aux_name);

          to_variable->reinit();

          // Determine number of DOFs that we're going to read and write
          auto && to_dof = to_variable->dofIndices();
          auto & from_values = from_variable->sln();

          // Check that the DOF matches
          if (from_variable->sln().size() != to_variable->sln().size())
            mooseError("Order of SCALAR variables do not match for sending and "
                       "receiving data for the "
                       "MultiAppScalarToAuxScalarTransfer!");

          for (MooseIndex(from_values) j = 0; j < from_values.size(); ++j)
            to_variable->sys().solution().set(to_dof[j], from_values[j]);

          to_variable->sys().solution().close();
        }
      break;
    }

    // multi_app to main app
    case FROM_MULTIAPP:
    {
      // The AuxVariable that will be modified with data from the subapp
      MooseVariableScalar * to_variable =
          &getFromMultiApp()->problemBase().getScalarVariable(_tid, _to_aux_name);

      // Ensure that the variable is up to date
      to_variable->reinit();

      // The dof indices for the scalar variable of interest
      auto && to_dof = to_variable->dofIndices();

      // Loop over each sub-app and populate the AuxVariable values
      for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
      {
        if (getFromMultiApp()->hasLocalApp(i) && getFromMultiApp()->isRootProcessor())
        {
          // Extract the scalar variable that is being transferred
          FEProblemBase & from_problem = getFromMultiApp()->appProblemBase(i);
          MooseVariableScalar * from_variable =
              &from_problem.getScalarVariable(_tid, _from_variable_name);

          // Loop over the scalar aux variable that we're going to write
          auto & from_values = from_variable->sln();

          // Check that DOFs match
          if (from_variable->sln().size() != to_variable->sln().size())
            mooseError("Order of SCALAR variables do not match for sending and "
                       "receiving data for the "
                       "MultiAppScalarToAuxScalarTransfer!");

          for (MooseIndex(from_values) j = 0; j < from_values.size(); ++j)
            to_variable->sys().solution().set(to_dof[j], from_values[j]);
        }
      }
      to_variable->sys().solution().close();

      break;
    }

    // multi_app to multi_app
    case BETWEEN_MULTIAPP:
    {
      for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
      {
        if (getFromMultiApp()->hasLocalApp(i))
        {
          // The AuxVariable that will be modified with data from the subapp
          MooseVariableScalar * from_variable =
              &getFromMultiApp()->appProblemBase(i).getScalarVariable(_tid, _from_variable_name);

          // Ensure that the variable is up to date
          from_variable->reinit();

          if (getToMultiApp()->hasLocalApp(i))
          {
            // Get reference to the scalar variable that will be written
            MooseVariableScalar * to_variable =
                &getToMultiApp()->appProblemBase(i).getScalarVariable(_tid, _to_aux_name);

            to_variable->reinit();

            // Determine number of DOFs that we're going to read and write
            auto && to_dof = to_variable->dofIndices();
            auto & from_values = from_variable->sln();

            // Check that the DOF matches
            if (from_variable->sln().size() != to_variable->sln().size())
              mooseError("Order of SCALAR variables do not match for sending and "
                         "receiving data for the "
                         "MultiAppScalarToAuxScalarTransfer!");

            for (MooseIndex(from_values) k = 0; k < from_values.size(); ++k)
              to_variable->sys().solution().set(to_dof[k], from_values[k]);

            to_variable->sys().solution().close();
          }
        }
      }
      break;
    }
  }
}
