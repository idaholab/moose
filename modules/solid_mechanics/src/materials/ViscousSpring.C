//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ViscousSpring.h"

registerMooseObject("SolidMechanicsApp", ViscousSpring);

InputParameters
ViscousSpring::validParams()
{
  InputParameters params = SpringLawBase::validParams();
  params.addClassDescription("Viscous spring law, a dashpot: the force along each local axis is "
                             "a constant times the rate of the deformation along that axis.");
  params.addRequiredParam<RealVectorValue>(
      "damping",
      "The three damping coefficients along local axes 1, 2 and 3, each zero or positive. "
      "Dimension force times time per length (moment times time per radian in the rotational "
      "mode), divided by the dimension of tributary_area when an area is given. The "
      "deformation rates come from ComputeSpringDeformation with compute_rates = true.");
  return params;
}

ViscousSpring::ViscousSpring(const InputParameters & parameters)
  : SpringLawBase(parameters),
    _damping(getParam<RealVectorValue>("damping")),
    _rate(getMaterialPropertyByName<RealVectorValue>(
        _base_name + (_rotational ? "spring_rotation_rate" : "spring_deformation_rate"))),
    _rate_du_dot_du(getMaterialPropertyByName<Real>(_base_name + "spring_rate_du_dot_du"))
{
  for (unsigned int axis = 0; axis < 3; ++axis)
    if (_damping(axis) < 0.0)
      paramError("damping", "The damping coefficients must be zero or positive.");
}

void
ViscousSpring::computeLaw(const RealVectorValue & /*deformation*/,
                          RealVectorValue & force,
                          RankTwoTensor & stiffness)
{
  stiffness.zero();
  for (unsigned int axis = 0; axis < 3; ++axis)
  {
    force(axis) = _damping(axis) * _rate[_qp](axis);
    stiffness(axis, axis) = _damping(axis) * _rate_du_dot_du[_qp];
  }
}
