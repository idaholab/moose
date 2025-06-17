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
AbaqusEssentialBC::residualSetup()
{
  _current_step_end_values = _step_uo.getEndValues(_abaqus_var_id);
  _current_step_begin_solution = _step_uo.getBeginSolution(_abaqus_var_id);
  _current_step_begin_values = _step_uo.getBeginValues(_abaqus_var_id);
}

bool
AbaqusEssentialBC::shouldApply() const
{
  const auto id = _current_node->id();

  const auto * cse = _current_step_end_values;
  if (cse == nullptr || cse->find(id) == cse->end())
    return false;

  const auto * cbs = _current_step_begin_solution;
  if (cbs && cbs->find(id) != cbs->end())
    return true;

  const auto * csb = _current_step_begin_values;
  if (csb && csb->find(id) != csb->end())
    return true;

  return false;
}

Real
AbaqusEssentialBC::computeQpResidual()
{
  const auto id = _current_node->id();
  const Real & d = _current_step_fraction;
  Real end_value = _current_step_end_values->at(id);

  const auto * cbs = _current_step_begin_solution;
  if (cbs && cbs->find(id) != cbs->end())
  {
    // BC is being actibated during this step
    const auto begin_value = cbs->at(id);
    const Real value = d * end_value + (1.0 - d) * begin_value;
    return _u[_qp] - value;
  }

  const auto * csb = _current_step_begin_values;
  if (csb&& csb->find(id) != csb->end())
  {
    // BC is staying in effect (but maybe changing value)
    const auto begin_value = csb->at(id);
    const Real value = d * end_value + (1.0 - d) * begin_value;
    return _u[_qp] - value;
  }

  mooseError("ShouldApply should have returned false!");
}
