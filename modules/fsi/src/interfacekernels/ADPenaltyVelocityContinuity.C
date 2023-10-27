//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPenaltyVelocityContinuity.h"
#include "Assembly.h"

registerMooseObject("FsiApp", ADPenaltyVelocityContinuity);

InputParameters
ADPenaltyVelocityContinuity::validParams()
{
  InputParameters params = InterfaceKernelBase::validParams();
  params.addClassDescription(
      "Enforces continuity of flux and continuity of solution via penalty across an interface.");
  params.addRequiredParam<Real>(
      "penalty",
      "The penalty that penalizes jump between primary and neighbor secondary variables.");
  params.addRequiredCoupledVar("fluid_velocity", "The fluid velocity");
  params.renameCoupledVar("neighbor_var", "displacements", "All the displacement variables");
  params.addRequiredCoupledVar("solid_velocities", "The solid velocity components");
  return params;
}

ADPenaltyVelocityContinuity::ADPenaltyVelocityContinuity(const InputParameters & parameters)
  : InterfaceKernelBase(parameters),
    _penalty(getParam<Real>("penalty")),
    _velocity_var(getVectorVar("fluid_velocity", 0)),
    _velocity(adCoupledVectorValue("fluid_velocity")),
    _ad_JxW(_assembly.adJxWFace()),
    _ad_coord(_assembly.adCoordTransformation())
{
  if (isCoupledConstant("fluid_velocity"))
    paramError("fluid_velocity", "The fluid velocity must be an actual variable");

  _solid_velocities.resize(coupledComponents("solid_velocities"));
  for (const auto i : index_range(_solid_velocities))
    _solid_velocities[i] = &adCoupledNeighborValue("solid_velocities", i);

  _displacements.resize(coupledComponents("displacements"));
  for (const auto i : index_range(_displacements))
    _displacements[i] = getVar("displacements", i);
}

void
ADPenaltyVelocityContinuity::computeResidual()
{
  _qp_jumps.resize(_qrule->n_points());
  for (auto & qp_jump : _qp_jumps)
    qp_jump = 0;

  const auto solid_velocity = [&](const auto qp)
  {
    ADRealVectorValue ret;
    for (const auto i : index_range(_solid_velocities))
      if (_solid_velocities[i])
        ret(i) = (*_solid_velocities[i])[qp];

    return ret;
  };

  for (const auto qp : make_range(_qrule->n_points()))
    _qp_jumps[qp] = _ad_JxW[qp] * _ad_coord[qp] * _penalty * (_velocity[qp] - solid_velocity(qp));

  // Fluid velocity residuals
  {
    const auto & phi = _velocity_var->phiFace();
    const auto & dof_indices = _velocity_var->dofIndices();
    mooseAssert(phi.size() == dof_indices.size(), "These should be the same");

    _residuals.resize(phi.size());
    for (auto & r : _residuals)
      r = 0;

    for (const auto i : index_range(phi))
      for (const auto qp : make_range(_qrule->n_points()))
        _residuals[i] += phi[i][qp] * _qp_jumps[qp];

    addResidualsAndJacobian(_assembly, _residuals, dof_indices, _velocity_var->scalingFactor());
  }

  // Displacement residuals
  for (const auto dim : index_range(_displacements))
  {
    const auto * const disp_var = _displacements[dim];
    if (disp_var)
    {
      const auto & phi = disp_var->phiFaceNeighbor();
      const auto & dof_indices = disp_var->dofIndicesNeighbor();
      mooseAssert(phi.size() == dof_indices.size(), "These should be the same");

      _residuals.resize(phi.size());
      for (auto & r : _residuals)
        r = 0;

      for (const auto qp : make_range(_qrule->n_points()))
        for (const auto i : index_range(phi))
          _residuals[i] -= phi[i][qp] * _qp_jumps[qp](dim);

      addResidualsAndJacobian(_assembly, _residuals, dof_indices, disp_var->scalingFactor());
    }
  }
}
