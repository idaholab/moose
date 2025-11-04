//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPenaltyVelocityContinuityNewmarkBeta.h"
#include "InertialForce.h"

registerMooseObject("FsiApp", ADPenaltyVelocityContinuityNewmarkBeta);

InputParameters
ADPenaltyVelocityContinuityNewmarkBeta::validParams()
{
  InputParameters params = ADPenaltyVelocityContinuity::validParams();
  params.addClassDescription(
      "Enforces continuity of flux and continuity of solution via penalty across an interface with "
      "the solid velocity computed via the Newmark-Beta method.");
  params.setDocString(
      "solid_velocities",
      "solid velocity variables whose previous timestep values we will use to compute the current "
      "solid velocities using the Newmark-Beta time integration method");
  params.addRequiredCoupledVar(
      "solid_accelerations",
      "solid acceleration variables whose previous timestep values we will use to compute the "
      "current solid accelerations using the Newmark-Beta time integration method");
  params.addRequiredParam<Real>("beta", "beta parameter for Newmark-Beta time integration");
  params.addRequiredParam<Real>("gamma", "gamma parameter for Newmark-Beta time integration");
  return params;
}

ADPenaltyVelocityContinuityNewmarkBeta::ADPenaltyVelocityContinuityNewmarkBeta(
    const InputParameters & parameters)
  : ADPenaltyVelocityContinuity(parameters),
    _beta(getParam<Real>("beta")),
    _gamma(getParam<Real>("gamma"))
{
  _solid_velocities_old.resize(coupledComponents("solid_velocities"));
  for (const auto i : index_range(_solid_velocities_old))
    _solid_velocities_old[i] = &coupledValueOld("solid_velocities", i);

  _solid_accelerations_old.resize(coupledComponents("solid_accelerations"));
  for (const auto i : index_range(_solid_accelerations_old))
    _solid_accelerations_old[i] = &coupledValueOld("solid_accelerations", i);

  if (_solid_accelerations_old.size() != _solid_velocities_old.size())
    paramError("solid_velocities",
               "'solid_velocities' and 'solid_accelerations' must be the same length!");

  _displacement_values.resize(_displacements.size());
  _displacement_values_old.resize(_displacements.size());
  for (const auto i : index_range(_displacement_values))
  {
    _displacement_values[i] = &_displacements[i]->adSln();
    _displacement_values_old[i] = &_displacements[i]->slnOld();
  }
}

ADRealVectorValue
ADPenaltyVelocityContinuityNewmarkBeta::solidVelocity(const unsigned int qp) const
{
  ADRealVectorValue ret;
  for (const auto i : index_range(_solid_velocities_old))
    ret(i) =
        InertialForceTempl<true>::computeNewmarkBetaVelAccel((*_displacement_values[i])[qp],
                                                             (*_displacement_values_old[i])[qp],
                                                             (*_solid_velocities_old[i])[qp],
                                                             (*_solid_accelerations_old[i])[qp],
                                                             _beta,
                                                             _gamma,
                                                             _dt)
            .first;
  return ret;
}
