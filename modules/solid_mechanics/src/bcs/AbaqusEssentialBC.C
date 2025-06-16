//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusEssentialBC.h"
#include "AbaqusUELStepUserObject.h"
#include "FEProblem.h"
#include "libmesh/libmesh_common.h"

registerMooseObject("SolidMechanicsApp", AbaqusEssentialBC);

InputParameters
AbaqusEssentialBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addClassDescription(
      "Applies boundary conditions from an Abaqus input read through AbaqusUELMesh");
  params.addRequiredParam<Abaqus::AbaqusID>("abaqus_var_id", "Abaqus variable (DOF) id number.");
  params.addRequiredParam<UserObjectName>("step_user_object", "Step user object");
  // we generate a union of all nodes where any BC applies and suss in the individual case
  // if we really need to set a value for the current variable (shouldApply)
  params.set<std::vector<BoundaryName>>("boundary") = {"abaqus_bc_union_boundary"};
  return params;
}

AbaqusEssentialBC::AbaqusEssentialBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _step_uo(getUserObject<AbaqusUELStepUserObject>("step_user_object")),
    _abaqus_var_id(getParam<Abaqus::AbaqusID>("abaqus_var_id")),
    _current_step_fraction(_step_uo.getStepFraction())
{
}

void
AbaqusEssentialBC::timestepSetup()
{
  _current_step_begin_forces = _step_uo.getBeginForces(_abaqus_var_id);
  _current_step_begin_solution = _step_uo.getBeginSolution(_abaqus_var_id);
  _current_step_begin_values = _step_uo.getBeginValues(_abaqus_var_id);
  _current_step_end_values = _step_uo.getEndValues(_abaqus_var_id);
}

void
AbaqusEssentialBC::computeResidual()
{
  const auto id = _current_node->id();
  const Real & d = _current_step_fraction;

  const auto * cbf = _current_step_begin_forces;
  if (cbf)
  {
    if (const auto it = cbf->find(id); it != cbf->end())
    {
      std::cout << "Add residual node " << id << " on (1.0 - " << d << ')' << " * " << it->second
                << " = " << ((1.0 - d) * it->second) << '\n';
      addResidual(_sys, (1.0 - d) * it->second, _var);
      return;
    }
  }

  Real end_value;
  const auto * cse = _current_step_end_values;
  if (cse)
  {
    const auto it = cse->find(id);
    if (it == cse->end())
      return;
    end_value = it->second;
  }

  const auto * cbs = _current_step_begin_solution;
  if (cbs)
  {
    if (const auto it = cbs->find(id); it != cbs->end())
    {
      const Real value = d * end_value + (1.0 - d) * it->second;
      std::cout << "Set residual node " << id << " to " << _u[_qp] << "-" << value << " = "
                << (_u[_qp] - value) << '\n';
      setResidual(_sys, _u[_qp] - value, _var);
      return;
    }
  }

  const auto * csb = _current_step_begin_values;
  if (csb)
  {
    if (const auto it = csb->find(id); it != cse->end())
    {
      // BC is staying in effect (but maybe changing value)
      const Real value = d * end_value + (1.0 - d) * it->second;
      std::cout << "Set residual node " << id << " to " << d << " * " << end_value << " + (1.0 - "
                << d << ')' << " * " << it->second << " = " << (_u[_qp] - value) << '\n';
      setResidual(_sys, _u[_qp] - value, _var);
      return;
    }
  }
  std::cout << "Ignore node\n";
}

Real
AbaqusEssentialBC::computeQpJacobian()
{
  return 1.0;
  const auto id = _current_node->id();

  const auto * cbf = _current_step_begin_forces;
  if (cbf && cbf->find(id) != cbf->end())
    // if the BC is deactivated it's force does not depend on the current solution
    return 0.0;

  const auto * cse = _current_step_end_values;
  if (cse && cse->find(id) == cse->end())
    // BC does not apply
    return 0.0;

  const auto * cbs = _current_step_begin_solution;
  if (cbs && cbs->find(id) != cbs->end())
    // linear dependence (going from a solution at the beginning of the step to a prescribed BC
    // value)
    return 1.0;

  const auto * csb = _current_step_begin_values;
  if (csb && csb->find(id) == cse->end())
    // BC does not apply
    return 0.0;

  // linear dependence (going from one prescribed value to the next)
  return 1.0;
}
