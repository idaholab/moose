//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InertialForceDSGShell.h"

registerMooseObject("SolidMechanicsApp", InertialForceDSGShell);

InputParameters
InertialForceDSGShell::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params.addClassDescription("Calculates the residual for the inertial force or moment of one "
                             "degree of freedom of a DSG shell element, with the contribution of "
                             "mass-proportional Rayleigh damping and of the HHT time integration "
                             "scheme.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component",
      "component < 6",
      "The degree of freedom this kernel assembles: 0, 1 and 2 for the displacements along the "
      "global axes X, Y and Z, 3, 4 and 5 for the rotations about them. Six kernels are needed, "
      "one per variable, and the variable of each must be the one listed at that position of "
      "'displacements' and 'rotations'.");
  params.addRequiredCoupledVar(
      "displacements",
      "The three displacement variables of the shell nodes, in the order of the global axes X, Y "
      "and Z, for example 'disp_x disp_y disp_z'. Every object of the DSG shell family takes the "
      "same list.");
  params.addRequiredCoupledVar(
      "rotations",
      "The three rotation variables of the shell nodes, the rotations about the global axes X, Y "
      "and Z in that order, for example 'rot_x rot_y rot_z'. The rotation about the normal of an "
      "element is its drilling rotation.");
  params.addParam<MaterialPropertyName>(
      "density",
      "density",
      "Name of the material property that gives the mass density of the shell material, a mass "
      "per unit volume (the mass per unit area of the shell is the density times the "
      "thickness). The property can be declared with GenericConstantMaterial.");
  params.addParam<bool>(
      "rotary_inertia",
      true,
      "Whether the two rotations in the plane of each element carry the rotary inertia of the "
      "thickness, density * thickness^3 / 12 per unit area. Set to false to keep the "
      "translational mass only, which raises the frequencies slightly (0.1 percent on the first "
      "mode of a plate whose thickness is a fifteenth of its length). The drilling rotation "
      "carries no inertia either way.");
  params.addParam<MaterialPropertyName>(
      "eta",
      0.0,
      "The mass-proportional Rayleigh damping coefficient, given as a constant or as the name "
      "of a material property: the damping force is eta times the mass matrix times the "
      "velocity, so eta has the unit of one over time. Zero by default, no damping.");
  params.addRangeCheckedParam<Real>(
      "alpha",
      0.0,
      "alpha >= -0.3333 & alpha <= 0.0",
      "The alpha parameter of the HHT time integration scheme, between -1/3 and 0, which shifts "
      "the damping force toward the previous time step: eta * M * ((1 + alpha) * velocity - "
      "alpha * old velocity). Zero by default, the Newmark scheme without the shift. Use the "
      "same value as in the time integrator.");
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

InertialForceDSGShell::InertialForceDSGShell(const InputParameters & parameters)
  : TimeKernel(parameters),
    _component(getParam<unsigned int>("component")),
    _rotary_inertia(getParam<bool>("rotary_inertia")),
    _density(getMaterialProperty<Real>("density")),
    _thickness(getMaterialProperty<Real>("dsg_shell_thickness")),
    _rotation_matrix(getMaterialProperty<RankTwoTensor>("dsg_shell_rotation_matrix")),
    _eta(getMaterialProperty<Real>("eta")),
    _alpha(getParam<Real>("alpha"))
{
  if (coupledComponents("displacements") != 3 || coupledComponents("rotations") != 3)
    paramError("displacements",
               "'displacements' must list three variables and "
               "'rotations' must list three variables.");

  // The time derivatives of the six degrees of freedom, in the order of the components
  for (const char * group : {"displacements", "rotations"})
    for (unsigned int i = 0; i < 3; ++i)
    {
      _variable_numbers.push_back(coupled(group, i));
      _accelerations.push_back(&coupledDotDot(group, i));
      _velocities.push_back(&coupledDot(group, i));
      _old_velocities.push_back(&coupledDotOld(group, i));
      _acceleration_derivatives.push_back(&coupledDotDotDu(group, i));
      _velocity_derivatives.push_back(&coupledDotDu(group, i));
    }

  if (_variable_numbers[_component] != _var.number())
    paramError("component",
               "The variable of this kernel is not the one listed for component ",
               _component,
               " in 'displacements' and 'rotations'.");
}

Real
InertialForceDSGShell::dampedAcceleration(const unsigned int component) const
{
  // The acceleration, the Rayleigh damping on the velocity and the HHT shift of that damping
  // towards the previous velocity
  return (*_accelerations[component])[_qp] +
         _eta[_qp] * (1.0 + _alpha) * (*_velocities[component])[_qp] -
         _alpha * _eta[_qp] * (*_old_velocities[component])[_qp];
}

Real
InertialForceDSGShell::dampedAccelerationDerivative(const unsigned int component) const
{
  return (*_acceleration_derivatives[component])[_qp] +
         _eta[_qp] * (1.0 + _alpha) * (*_velocity_derivatives[component])[_qp];
}

Real
InertialForceDSGShell::inPlaneProjector(const unsigned int axis_a, const unsigned int axis_b) const
{
  // The rotary inertia acts on the rotations about the two axes in the plane of the element.
  // In the global frame that is the identity minus the outer product of the element normal,
  // which is the third row of the rotation matrix.
  const RankTwoTensor & rotation = _rotation_matrix[_qp];
  const Real identity = (axis_a == axis_b) ? 1.0 : 0.0;
  return identity - rotation(2, axis_a) * rotation(2, axis_b);
}

Real
InertialForceDSGShell::computeQpResidual()
{
  // A zero time step is a steady or an initial residual evaluation, with no inertia
  if (_dt == 0)
    return 0.0;

  const Real thickness = _thickness[_qp];

  // Translations: the mass per unit area of the mid-surface times the damped acceleration
  if (_component < 3)
    return _test[_i][_qp] * _density[_qp] * thickness * dampedAcceleration(_component);

  // Rotations: the rotary inertia per unit area on the in-plane rotations only
  if (!_rotary_inertia)
    return 0.0;

  const Real rotary_inertia = _density[_qp] * thickness * thickness * thickness / 12.0;
  const unsigned int axis = _component - 3;
  Real projected_acceleration = 0.0;
  for (unsigned int other_axis = 0; other_axis < 3; ++other_axis)
    projected_acceleration +=
        inPlaneProjector(axis, other_axis) * dampedAcceleration(3 + other_axis);
  return _test[_i][_qp] * rotary_inertia * projected_acceleration;
}

Real
InertialForceDSGShell::computeQpJacobian()
{
  if (_dt == 0)
    return 0.0;

  const Real thickness = _thickness[_qp];

  if (_component < 3)
    return _test[_i][_qp] * _phi[_j][_qp] * _density[_qp] * thickness *
           dampedAccelerationDerivative(_component);

  if (!_rotary_inertia)
    return 0.0;

  const Real rotary_inertia = _density[_qp] * thickness * thickness * thickness / 12.0;
  const unsigned int axis = _component - 3;
  return _test[_i][_qp] * _phi[_j][_qp] * rotary_inertia * inPlaneProjector(axis, axis) *
         dampedAccelerationDerivative(_component);
}

Real
InertialForceDSGShell::computeQpOffDiagJacobian(unsigned int jvar)
{
  // Only the rotations couple with each other, through the in-plane projector
  if (_dt == 0 || _component < 3 || !_rotary_inertia)
    return 0.0;

  for (unsigned int other_axis = 0; other_axis < 3; ++other_axis)
  {
    const unsigned int other_component = 3 + other_axis;
    if (_variable_numbers[other_component] == jvar && other_component != _component)
    {
      const Real thickness = _thickness[_qp];
      const Real rotary_inertia = _density[_qp] * thickness * thickness * thickness / 12.0;
      return _test[_i][_qp] * _phi[_j][_qp] * rotary_inertia *
             inPlaneProjector(_component - 3, other_axis) *
             dampedAccelerationDerivative(other_component);
    }
  }
  return 0.0;
}
