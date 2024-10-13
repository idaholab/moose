//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppVariableValueSampleTransfer.h"

// MOOSE includes
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "MultiApp.h"
#include "SystemBase.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/system.h"

using namespace libMesh;

registerMooseObject("MooseApp", MultiAppVariableValueSampleTransfer);

InputParameters
MultiAppVariableValueSampleTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addClassDescription(
      "Transfers the value of a variable within the master application at each sub-application "
      "position and transfers the value to a field variable on the sub-application(s).");
  params.addRequiredParam<AuxVariableName>(
      "variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  return params;
}

MultiAppVariableValueSampleTransfer::MultiAppVariableValueSampleTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable"))
{
  if (_directions.size() != 1 || (isParamValid("from_multi_app") && isParamValid("to_multi_app")))
    paramError("direction", "This transfer is only unidirectional");

  if (hasFromMultiApp())
    paramError("from_multi_app", "This transfer direction has not been implemented");
}

void
MultiAppVariableValueSampleTransfer::initialSetup()
{
  MultiAppTransfer::initialSetup();

  variableIntegrityCheck(_to_var_name);

  if (isParamValid("from_multi_app"))
    getFromMultiApp()->problemBase().mesh().errorIfDistributedMesh(
        "MultiAppVariableValueSampleTransfer");
  if (isParamValid("to_multi_app"))
    getToMultiApp()->problemBase().mesh().errorIfDistributedMesh(
        "MultiAppVariableValueSampleTransfer");
}

void
MultiAppVariableValueSampleTransfer::execute()
{
  TIME_SECTION(
      "MultiAppVariableValueSampleTransfer::execute()", 5, "Sampling a variable for transfer");

  switch (_current_direction)
  {
    case TO_MULTIAPP:
    {
      FEProblemBase & from_problem = getToMultiApp()->problemBase();
      MooseVariableField<Real> & from_var = static_cast<MooseVariableField<Real> &>(
          from_problem.getActualFieldVariable(0, _from_var_name));
      SystemBase & from_system_base = from_var.sys();
      SubProblem & from_sub_problem = from_system_base.subproblem();

      MooseMesh & from_mesh = from_problem.mesh();

      std::unique_ptr<PointLocatorBase> pl = from_mesh.getPointLocator();

      for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
      {
        Real value = -std::numeric_limits<Real>::max();

        { // Get the value of the variable at the point where this multiapp is in the master domain

          Point multi_app_position = getToMultiApp()->position(i);

          std::vector<Point> point_vec(1, multi_app_position);

          // First find the element the hit lands in
          const Elem * elem = (*pl)(multi_app_position);

          if (elem && elem->processor_id() == from_mesh.processor_id())
          {
            from_sub_problem.setCurrentSubdomainID(elem, 0);
            from_sub_problem.reinitElemPhys(elem, point_vec, 0);

            mooseAssert(from_var.sln().size() == 1, "No values in u!");
            value = from_var.sln()[0];
          }

          _communicator.max(value);

          if (value == -std::numeric_limits<Real>::max())
            mooseError("Transfer failed to sample point value at point: ", multi_app_position);
        }

        if (getToMultiApp()->hasLocalApp(i))
        {
          Moose::ScopedCommSwapper swapper(getToMultiApp()->comm());

          // Loop over the master nodes and set the value of the variable
          System * to_sys = find_sys(getToMultiApp()->appProblemBase(i).es(), _to_var_name);

          unsigned int sys_num = to_sys->number();
          unsigned int var_num = to_sys->variable_number(_to_var_name);

          NumericVector<Real> & solution = getToMultiApp()->appTransferVector(i, _to_var_name);

          MooseMesh & mesh = getToMultiApp()->appProblemBase(i).mesh();

          for (const auto & node : as_range(mesh.localNodesBegin(), mesh.localNodesEnd()))
          {
            if (node->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this node
            {
              // The zero only works for LAGRANGE!
              dof_id_type dof = node->dof_number(sys_num, var_num, 0);

              solution.set(dof, value);
            }
          }
          solution.close();
          getToMultiApp()->appProblemBase(i).es().update();
        }
      }

      break;
    }
    case FROM_MULTIAPP:
    {
      mooseError("Doesn't make sense to transfer a sampled variable's value from a MultiApp!!");
      break;
    }
  }
}
