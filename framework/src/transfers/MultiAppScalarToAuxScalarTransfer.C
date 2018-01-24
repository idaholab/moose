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
template <>
InputParameters
validParams<MultiAppScalarToAuxScalarTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<VariableName>("source_variable",
                                        "The name of the scalar variable in the MultiApp to "
                                        "transfer the value from.");
  params.addRequiredParam<VariableName>("to_aux_scalar",
                                        "The name of the scalar Aux variable in the MultiApp to "
                                        "transfer the value to.");
  return params;
}

MultiAppScalarToAuxScalarTransfer::MultiAppScalarToAuxScalarTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _from_variable_name(getParam<VariableName>("source_variable")),
    _to_aux_name(getParam<VariableName>("to_aux_scalar"))
{
}

void
MultiAppScalarToAuxScalarTransfer::execute()
{
  _console << "Beginning ScalarToAuxScalarTransfer " << name() << std::endl;

  // Perform action based on the transfer direction
  switch (_direction)
  {
    // MasterApp -> SubApp
    case TO_MULTIAPP:
    {
      // Extract the scalar variable that is being transferred
      FEProblemBase & from_problem = _multi_app->problemBase();
      MooseVariableScalar * from_variable =
          &from_problem.getScalarVariable(_tid, _from_variable_name);

      // Loop through each of the sub apps
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
        if (_multi_app->hasLocalApp(i))
        {
          // Get reference to the scalar variable that will be written
          MooseVariableScalar * to_variable =
              &_multi_app->appProblemBase(i).getScalarVariable(_tid, _to_aux_name);

          to_variable->reinit();

          // Determine number of DOFs that we're going to read and write
          std::vector<dof_id_type> & to_dof = to_variable->dofIndices();
          auto & from_values = from_variable->sln();

          // Check that the DOF matches
          if (from_variable->sln().size() != to_variable->sln().size())
            mooseError("Order of SCALAR variables do not match for sending and "
                       "receiving data for the "
                       "MultiAppScalarToAuxScalarTransfer!");

          for (auto j = beginIndex(from_values); j < from_values.size(); ++j)
            to_variable->sys().solution().set(to_dof[j], from_values[j]);

          to_variable->sys().solution().close();
        }
      break;
    }

    // SubApp -> MasterApp
    case FROM_MULTIAPP:
    {
      // The AuxVariable that will be read from the subApp
      MooseVariableScalar * to_variable =
          &_multi_app->problemBase().getScalarVariable(_tid, _to_aux_name);

      // Ensure that the variable is up to date
      to_variable->reinit();

      // The dof indices for the scalar variable of interest
      std::vector<dof_id_type> & to_dof = to_variable->dofIndices();

      // Loop over each sub-app and populate the AuxVariable values
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      {
        if (_multi_app->hasLocalApp(i) && _multi_app->isRootProcessor())
        {
          // Extract the scalar variable that is being transferred
          FEProblemBase & from_problem = _multi_app->appProblemBase(i);
          MooseVariableScalar * from_variable =
              &from_problem.getScalarVariable(_tid, _from_variable_name);

          // Loop over the scalar aux variable that we're going to write
          auto & from_values = from_variable->sln();

          // Check that DOFs match
          if (from_variable->sln().size() != to_variable->sln().size())
            mooseError("Order of SCALAR variables do not match for sending and "
                       "receiving data for the "
                       "MultiAppScalarToAuxScalarTransfer!");

          for (auto j = beginIndex(from_values); j < from_values.size(); ++j)
            to_variable->sys().solution().set(to_dof[j], from_values[j]);
        }
      }
      to_variable->sys().solution().close();

      break;
    }
  }

  _console << "Finished ScalarToAuxScalarTransfer " << name() << std::endl;
}
