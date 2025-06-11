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

Real
AbaqusEssentialBC::computeQpResidual()
{
  const Real & d = _current_step_fraction;

  // BC is deactivated over the course of this step
  const auto deactivated_it = _current_step_begin_forces->find(_current_node->id());
  if (deactivated_it != _current_step_begin_forces->end())
    return (1.0 - d) * deactivated_it->second;

  const auto end_it = _current_step_end_values->find(_current_node->id());
  if (end_it == _current_step_end_values->end())
    mooseError("Missing step end value.");

  // new BC is coming into effect this step
  const auto activated_it = _current_step_begin_solution->find(_current_node->id());
  if (activated_it != _current_step_begin_solution->end())
  {
    const Real value = d * end_it->second + (1.0 - d) * activated_it->second;
    return _u[_qp] - value;
  }

  const auto begin_it = _current_step_begin_values->find(_current_node->id());
  if (begin_it == _current_step_begin_values->end())
    mooseError("Missing step begin value.");

  // BC is staying in effect (but maybe changing value)
  const Real value = d * end_it->second + (1.0 - d) * begin_it->second;
  return _u[_qp] - value;
}

Real
AbaqusEssentialBC::computeQpJacobian()
{
  // if the BC is deactivated it's force does not depend on the current solution
  const auto deactivated_it = _current_step_begin_forces->find(_current_node->id());
  if (deactivated_it == _current_step_begin_forces->end())
    return 0.0;

  // otherwise it has a linear dependence
  return 1.0;
}
