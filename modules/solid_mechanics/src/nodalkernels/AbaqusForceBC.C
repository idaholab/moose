//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusForceBC.h"
#include "MooseUtils.h"
#include "DelimitedFileReader.h"
#include "Function.h"

registerMooseObject("SolidMechanicsApp", AbaqusForceBC);

InputParameters
AbaqusForceBC::validParams()
{
  InputParameters params = NodalKernel::validParams();
  params.addClassDescription("Applies a force to a node when an AbaqusEssentialBC is deactivated.");
  params.addRequiredParam<Abaqus::AbaqusID>("abaqus_var_id", "Abaqus variable (DOF) id number.");
  params.addRequiredParam<UserObjectName>("step_user_object", "Step user object");
  params.set<std::vector<BoundaryName>>("boundary") = {"abaqus_bc_union_boundary"};
  return params;
}

AbaqusForceBC::AbaqusForceBC(const InputParameters & parameters)
  : NodalKernel(parameters),
    _step_uo(getUserObject<AbaqusUELStepUserObject>("step_user_object")),
    _abaqus_var_id(getParam<Abaqus::AbaqusID>("abaqus_var_id")),
    _current_step_fraction(_step_uo.getStepFraction())

{
}

void
AbaqusForceBC::residualSetup()
{
  _current_step_begin_forces = _step_uo.getBeginForces(_abaqus_var_id);
}

Real
AbaqusForceBC::computeQpResidual()
{
  const auto id = _current_node->id();
  const Real & d = _current_step_fraction;

  const auto * cbf = _current_step_begin_forces;
  if (cbf)
  {
    if (const auto it = cbf->find(id); it != cbf->end())
      return (1.0 - d) * it->second;
  }

  return 0.0;
}
