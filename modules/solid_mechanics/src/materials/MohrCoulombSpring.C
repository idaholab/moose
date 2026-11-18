//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MohrCoulombSpring.h"

registerMooseObject("SolidMechanicsApp", MohrCoulombSpring);

InputParameters
MohrCoulombSpring::validParams()
{
  InputParameters params = SpringLawBase::validParams();
  params.addClassDescription("Interface spring law: compression along the normal, tension up to "
                             "a tensile strength, elastic shear capped by the Coulomb limit of "
                             "the normal stress; a separation may break the bond for good.");
  params.addRequiredRangeCheckedParam<Real>(
      "normal_stiffness",
      "normal_stiffness > 0",
      "Stiffness along local axis 1, in compression, and in tension while the bond holds "
      "(tensile_strength); without a tensile strength the interface carries no tension and "
      "opens freely. Dimension force per length, or force per length cubed (a subgrade modulus) "
      "when tributary_area is given.");
  params.addRequiredRangeCheckedParam<Real>(
      "shear_stiffness",
      "shear_stiffness > 0",
      "Elastic stiffness along local axes 2 and 3, the dimension of normal_stiffness, until "
      "the shear resultant reaches the Coulomb limit.");
  params.addRangeCheckedParam<Real>(
      "cohesion",
      0.0,
      "cohesion >= 0",
      "Shear strength at zero normal stress (an adhesion), carried until the interface separates. "
      "Dimension force, or force per area when tributary_area is given.");
  params.addRangeCheckedParam<Real>(
      "friction_angle",
      0.0,
      "friction_angle >= 0 & friction_angle < 90",
      "Friction angle in degrees (dimensionless): the shear limit grows by the compressive "
      "normal stress times its tangent.");
  params.addRangeCheckedParam<Real>(
      "initial_normal_stress",
      0.0,
      "initial_normal_stress >= 0",
      "Compressive normal stress on the interface before any deformation, the horizontal "
      "effective stress on a pile shaft for instance. It enters the Coulomb limit only, never "
      "the normal force of the spring (the soil carries it, not the spring), and an opening of "
      "the interface relaxes it: the limit uses initial_normal_stress - normal_stiffness times "
      "the opening, down to zero, where the interface has separated. Dimension force, or force "
      "per area when tributary_area is given, the dimension of cohesion.");
  params.addRangeCheckedParam<Real>(
      "tensile_strength",
      0.0,
      "tensile_strength >= 0",
      "Tensile strength of the bond on the interface, the dimension of cohesion. The normal "
      "stress may go into tension down to the tensile strength before the interface separates, "
      "the spring carrying that tension with normal_stiffness, and the shear limit falls "
      "linearly from the cohesion at zero normal stress to zero at the tensile strength, so "
      "that the bond and the shear capacity vanish together. Zero, the default, separates the "
      "interface at zero normal stress.");
  params.addParam<bool>(
      "permanent_breakage",
      false,
      "Whether a separation breaks the bond for good: afterwards the interface is frictional "
      "with residual_cohesion and residual_friction_angle and no tensile strength. False, the "
      "default, gives the bond back when the interface closes again.");
  params.addRangeCheckedParam<Real>(
      "residual_cohesion",
      0.0,
      "residual_cohesion >= 0",
      "Cohesion of the interface after a permanent breakage, the dimension of cohesion; zero by "
      "default. Needs permanent_breakage.");
  params.addRangeCheckedParam<Real>(
      "residual_friction_angle",
      "residual_friction_angle >= 0 & residual_friction_angle < 90",
      "Friction angle in degrees of the interface after a permanent breakage; the friction angle "
      "itself by default. Needs permanent_breakage.");
  params.set<MooseEnum>("mode") = "translational";
  params.suppressParameter<MooseEnum>("mode");
  return params;
}

MohrCoulombSpring::MohrCoulombSpring(const InputParameters & parameters)
  : SpringLawBase(parameters),
    _normal_stiffness(getParam<Real>("normal_stiffness")),
    _shear_stiffness(getParam<Real>("shear_stiffness")),
    _cohesion(getParam<Real>("cohesion")),
    _tan_friction_angle(std::tan(getParam<Real>("friction_angle") * libMesh::pi / 180.0)),
    _initial_normal_stress(getParam<Real>("initial_normal_stress")),
    _tensile_strength(getParam<Real>("tensile_strength")),
    _permanent_breakage(getParam<bool>("permanent_breakage")),
    _residual_cohesion(getParam<Real>("residual_cohesion")),
    _tan_residual_friction_angle(
        std::tan(getParam<Real>(isParamValid("residual_friction_angle") ? "residual_friction_angle"
                                                                        : "friction_angle") *
                 libMesh::pi / 180.0)),
    _plastic_slip(declareProperty<RealVectorValue>(name() + "_plastic_slip")),
    _plastic_slip_old(getMaterialPropertyOld<RealVectorValue>(name() + "_plastic_slip")),
    _bond_broken(declareProperty<Real>(name() + "_bond_broken")),
    _bond_broken_old(getMaterialPropertyOld<Real>(name() + "_bond_broken"))
{
  if (!_permanent_breakage)
  {
    if (isParamSetByUser("residual_cohesion"))
      paramError("residual_cohesion", "The residual cohesion needs permanent_breakage.");
    if (isParamValid("residual_friction_angle"))
      paramError("residual_friction_angle",
                 "The residual friction angle needs permanent_breakage.");
  }
}

void
MohrCoulombSpring::initQpStatefulProperties()
{
  _plastic_slip[_qp].zero();
  _bond_broken[_qp] = 0.0;
}

void
MohrCoulombSpring::computeLaw(const RealVectorValue & deformation,
                              RealVectorValue & force,
                              RankTwoTensor & stiffness)
{
  force.zero();
  stiffness.zero();

  // With permanent breakage the bond (cohesion, friction angle and tensile strength) holds
  // until the interface has separated once; the residual strength serves afterwards.
  const bool broken = _permanent_breakage && _bond_broken_old[_qp] > 0.5;
  const Real cohesion = broken ? _residual_cohesion : _cohesion;
  const Real tan_friction_angle = broken ? _tan_residual_friction_angle : _tan_friction_angle;
  const Real tensile_strength = broken ? 0.0 : _tensile_strength;

  // Compressive normal stress for the Coulomb limit: the initial stress relaxed by an opening
  // (positive deformation along axis 1) or raised by a closure. The spring's own normal force
  // is the closure times the stiffness in compression: the initial stress is carried by the
  // surrounding soil, not by the spring. In tension the spring carries the tensile part of
  // the normal stress, up to the tensile strength.
  const Real normal_stress = _initial_normal_stress - _normal_stiffness * deformation(0);
  if (normal_stress < -tensile_strength)
  {
    // Separated: no force at all, the breakage is recorded, and the shear slip follows the
    // deformation so that the shear restarts from zero when the interface closes again.
    _bond_broken[_qp] = 1.0;
    _plastic_slip[_qp] = RealVectorValue(0.0, deformation(1), deformation(2));
    return;
  }
  _bond_broken[_qp] = _bond_broken_old[_qp];

  if (deformation(0) < 0.0)
  {
    force(0) = _normal_stiffness * deformation(0);
    stiffness(0, 0) = _normal_stiffness;
  }
  else if (normal_stress < 0.0)
  {
    force(0) = -normal_stress;
    stiffness(0, 0) = _normal_stiffness;
  }

  // The Coulomb limit of the shear resultant: cohesion plus the compressive normal stress
  // times tan(friction angle); in tension it falls linearly from the cohesion to zero at the
  // tensile strength (a normal stress below zero here means the tensile strength is positive).
  Real shear_limit, limit_slope;
  if (normal_stress >= 0.0)
  {
    limit_slope = tan_friction_angle;
    shear_limit = cohesion + normal_stress * limit_slope;
  }
  else
  {
    limit_slope = cohesion / tensile_strength;
    shear_limit = cohesion + normal_stress * limit_slope;
  }

  // Elastic shear trial from the slip of the previous step, capped by the limit.
  RealVectorValue trial;
  for (unsigned int axis = 1; axis < 3; ++axis)
    trial(axis) = _shear_stiffness * (deformation(axis) - _plastic_slip_old[_qp](axis));
  const Real trial_norm = trial.norm();

  if (trial_norm <= shear_limit)
  {
    for (unsigned int axis = 1; axis < 3; ++axis)
    {
      force(axis) = trial(axis);
      stiffness(axis, axis) = _shear_stiffness;
    }
    _plastic_slip[_qp] = _plastic_slip_old[_qp];
    return;
  }

  // Sliding: the shear resultant sits on the limit in the direction of the trial, the slip
  // takes up the rest. The tangent has the radial return of the shear block and the change
  // of the limit with the normal deformation, which makes it non-symmetric.
  const RealVectorValue direction = trial / trial_norm;
  const Real limit_over_trial = shear_limit / trial_norm;
  for (unsigned int axis = 1; axis < 3; ++axis)
  {
    force(axis) = shear_limit * direction(axis);
    _plastic_slip[_qp](axis) = deformation(axis) - force(axis) / _shear_stiffness;
    for (unsigned int other = 1; other < 3; ++other)
      stiffness(axis, other) = _shear_stiffness * limit_over_trial *
                               ((axis == other ? 1.0 : 0.0) - direction(axis) * direction(other));
    stiffness(axis, 0) = -direction(axis) * _normal_stiffness * limit_slope;
  }
  _plastic_slip[_qp](0) = 0.0;
}
