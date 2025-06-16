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
AbaqusEssentialBC::computeResidual() const
{
  const auto id = _current_node->id();
  const Real & d = _current_step_fraction;

  const auto * cbf = _current_step_begin_forces;
  if (cbf)
  {
    if (const auto it = cbf->find(id); it != cbf->end())
    {
      addResidual(_sys, (1.0 - d) * it->second, _var);
      return;
    }
  }

  Real end_value;
  const auto * cse = _current_step_end_values;
  if (cse)
  {
    if (const auto it = cse->find(id); it == cse->end())
      return;
    end_value = it->second;
  }

  const auto * cbs = _current_step_begin_solution;
  if (cbs)
  {
    if (const auto it = cbs->find(id); it != cbs->end())
    {
      const Real value = d * end_value + (1.0 - d) * it->second;
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
      setResidual(_sys, _u[_qp] - value, _var);
      return;
    }
  }
}

Real
AbaqusEssentialBC::computeQpJacobian()
{
  // if the BC is deactivated it's force does not depend on the current solution
  if (_current_step_begin_forces)
  {
    const auto deactivated_it = _current_step_begin_forces->find(_current_node->id());
    if (deactivated_it == _current_step_begin_forces->end())
      return 0.0;
  }

  // otherwise it has a linear dependence
  return 1.0;
}
