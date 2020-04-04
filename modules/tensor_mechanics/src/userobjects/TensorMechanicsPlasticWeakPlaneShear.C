//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsPlasticWeakPlaneShear.h"
#include "RankFourTensor.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsPlasticWeakPlaneShear);

InputParameters
TensorMechanicsPlasticWeakPlaneShear::validParams()
{
  InputParameters params = TensorMechanicsPlasticModel::validParams();
  params.addRequiredParam<UserObjectName>(
      "cohesion",
      "A TensorMechanicsHardening UserObject that defines hardening of the cohesion.  "
      "Physically the cohesion should not be negative.");
  params.addRequiredParam<UserObjectName>("tan_friction_angle",
                                          "A TensorMechanicsHardening UserObject that defines "
                                          "hardening of tan(friction angle).  Physically the "
                                          "friction angle should be between 0 and 90deg.");
  params.addRequiredParam<UserObjectName>(
      "tan_dilation_angle",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "tan(dilation angle).  Usually the dilation angle is not greater than "
      "the friction angle, and it is between 0 and 90deg.");
  MooseEnum tip_scheme("hyperbolic cap", "hyperbolic");
  params.addParam<MooseEnum>(
      "tip_scheme", tip_scheme, "Scheme by which the cone's tip will be smoothed.");
  params.addRequiredRangeCheckedParam<Real>(
      "smoother",
      "smoother>=0",
      "For the 'hyperbolic' tip_scheme, the cone vertex at shear-stress "
      "= 0 will be smoothed by the given amount.  For the 'cap' "
      "tip_scheme, additional smoothing will occur.  Typical value is "
      "0.1*cohesion");
  params.addParam<Real>(
      "cap_start",
      0.0,
      "For the 'cap' tip_scheme, smoothing is performed in the stress_zz > cap_start region");
  params.addRangeCheckedParam<Real>("cap_rate",
                                    0.0,
                                    "cap_rate>=0",
                                    "For the 'cap' tip_scheme, this controls how quickly the cap "
                                    "degenerates to a hemisphere: small values mean a slow "
                                    "degeneration to a hemisphere (and zero means the 'cap' will "
                                    "be totally inactive).  Typical value is 1/cohesion");
  params.addClassDescription("Non-associative finite-strain weak-plane shear perfect plasticity.  "
                             "Here cohesion = 1, tan(phi) = 1 = tan(psi)");

  return params;
}

TensorMechanicsPlasticWeakPlaneShear::TensorMechanicsPlasticWeakPlaneShear(
    const InputParameters & parameters)
  : TensorMechanicsPlasticModel(parameters),
    _cohesion(getUserObject<TensorMechanicsHardeningModel>("cohesion")),
    _tan_phi(getUserObject<TensorMechanicsHardeningModel>("tan_friction_angle")),
    _tan_psi(getUserObject<TensorMechanicsHardeningModel>("tan_dilation_angle")),
    _tip_scheme(getParam<MooseEnum>("tip_scheme")),
    _small_smoother2(Utility::pow<2>(getParam<Real>("smoother"))),
    _cap_start(getParam<Real>("cap_start")),
    _cap_rate(getParam<Real>("cap_rate"))
{
  // With arbitary UserObjects, it is impossible to check everything, and
  // I think this is the best I can do
  if (tan_phi(0) < 0 || tan_psi(0) < 0)
    mooseError("Weak-Plane-Shear friction and dilation angles must lie in [0, Pi/2]");
  if (tan_phi(0) < tan_psi(0))
    mooseError(
        "Weak-Plane-Shear friction angle must not be less than Weak-Plane-Shear dilation angle");
  if (cohesion(0) < 0)
    mooseError("Weak-Plane-Shear cohesion must not be negative");
}

Real
TensorMechanicsPlasticWeakPlaneShear::yieldFunction(const RankTwoTensor & stress, Real intnl) const
{
  // note that i explicitly symmeterise in preparation for Cosserat
  return std::sqrt(Utility::pow<2>((stress(0, 2) + stress(2, 0)) / 2.0) +
                   Utility::pow<2>((stress(1, 2) + stress(2, 1)) / 2.0) + smooth(stress)) +
         stress(2, 2) * tan_phi(intnl) - cohesion(intnl);
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneShear::df_dsig(const RankTwoTensor & stress,
                                              Real _tan_phi_or_psi) const
{
  RankTwoTensor deriv; // the constructor zeroes this

  Real tau = std::sqrt(Utility::pow<2>((stress(0, 2) + stress(2, 0)) / 2.0) +
                       Utility::pow<2>((stress(1, 2) + stress(2, 1)) / 2.0) + smooth(stress));
  // note that i explicitly symmeterise in preparation for Cosserat
  if (tau == 0.0)
  {
    // the derivative is not defined here, but i want to set it nonzero
    // because otherwise the return direction might be too crazy
    deriv(0, 2) = deriv(2, 0) = 0.5;
    deriv(1, 2) = deriv(2, 1) = 0.5;
  }
  else
  {
    deriv(0, 2) = deriv(2, 0) = 0.25 * (stress(0, 2) + stress(2, 0)) / tau;
    deriv(1, 2) = deriv(2, 1) = 0.25 * (stress(1, 2) + stress(2, 1)) / tau;
    deriv(2, 2) = 0.5 * dsmooth(stress) / tau;
  }
  deriv(2, 2) += _tan_phi_or_psi;
  return deriv;
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneShear::dyieldFunction_dstress(const RankTwoTensor & stress,
                                                             Real intnl) const
{
  return df_dsig(stress, tan_phi(intnl));
}

Real
TensorMechanicsPlasticWeakPlaneShear::dyieldFunction_dintnl(const RankTwoTensor & stress,
                                                            Real intnl) const
{
  return stress(2, 2) * dtan_phi(intnl) - dcohesion(intnl);
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneShear::flowPotential(const RankTwoTensor & stress, Real intnl) const
{
  return df_dsig(stress, tan_psi(intnl));
}

RankFourTensor
TensorMechanicsPlasticWeakPlaneShear::dflowPotential_dstress(const RankTwoTensor & stress,
                                                             Real /*intnl*/) const
{
  RankFourTensor dr_dstress;
  Real tau = std::sqrt(Utility::pow<2>((stress(0, 2) + stress(2, 0)) / 2.0) +
                       Utility::pow<2>((stress(1, 2) + stress(2, 1)) / 2.0) + smooth(stress));
  if (tau == 0.0)
    return dr_dstress;

  // note that i explicitly symmeterise
  RankTwoTensor dtau;
  dtau(0, 2) = dtau(2, 0) = 0.25 * (stress(0, 2) + stress(2, 0)) / tau;
  dtau(1, 2) = dtau(2, 1) = 0.25 * (stress(1, 2) + stress(2, 1)) / tau;
  dtau(2, 2) = 0.5 * dsmooth(stress) / tau;

  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
      for (unsigned k = 0; k < 3; ++k)
        for (unsigned l = 0; l < 3; ++l)
          dr_dstress(i, j, k, l) = -dtau(i, j) * dtau(k, l) / tau;

  // note that i explicitly symmeterise
  dr_dstress(0, 2, 0, 2) += 0.25 / tau;
  dr_dstress(0, 2, 2, 0) += 0.25 / tau;
  dr_dstress(2, 0, 0, 2) += 0.25 / tau;
  dr_dstress(2, 0, 2, 0) += 0.25 / tau;
  dr_dstress(1, 2, 1, 2) += 0.25 / tau;
  dr_dstress(1, 2, 2, 1) += 0.25 / tau;
  dr_dstress(2, 1, 1, 2) += 0.25 / tau;
  dr_dstress(2, 1, 2, 1) += 0.25 / tau;
  dr_dstress(2, 2, 2, 2) += 0.5 * d2smooth(stress) / tau;

  return dr_dstress;
}

RankTwoTensor
TensorMechanicsPlasticWeakPlaneShear::dflowPotential_dintnl(const RankTwoTensor & /*stress*/,
                                                            Real intnl) const
{
  RankTwoTensor dr_dintnl;
  dr_dintnl(2, 2) = dtan_psi(intnl);
  return dr_dintnl;
}

Real
TensorMechanicsPlasticWeakPlaneShear::cohesion(const Real internal_param) const
{
  return _cohesion.value(internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneShear::dcohesion(const Real internal_param) const
{
  return _cohesion.derivative(internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneShear::tan_phi(const Real internal_param) const
{
  return _tan_phi.value(internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneShear::dtan_phi(const Real internal_param) const
{
  return _tan_phi.derivative(internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneShear::tan_psi(const Real internal_param) const
{
  return _tan_psi.value(internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneShear::dtan_psi(const Real internal_param) const
{
  return _tan_psi.derivative(internal_param);
}

Real
TensorMechanicsPlasticWeakPlaneShear::smooth(const RankTwoTensor & stress) const
{
  Real smoother2 = _small_smoother2;
  if (_tip_scheme == "cap")
  {
    Real x = stress(2, 2) - _cap_start;
    Real p = 0;
    if (x > 0)
      p = x * (1 - std::exp(-_cap_rate * x));
    smoother2 += Utility::pow<2>(p);
  }
  return smoother2;
}

Real
TensorMechanicsPlasticWeakPlaneShear::dsmooth(const RankTwoTensor & stress) const
{
  Real dsmoother2 = 0;
  if (_tip_scheme == "cap")
  {
    Real x = stress(2, 2) - _cap_start;
    Real p = 0;
    Real dp_dx = 0;
    if (x > 0)
    {
      const Real exp_cap_rate_x = std::exp(-_cap_rate * x);
      p = x * (1 - exp_cap_rate_x);
      dp_dx = (1 - exp_cap_rate_x) + x * _cap_rate * exp_cap_rate_x;
    }
    dsmoother2 += 2 * p * dp_dx;
  }
  return dsmoother2;
}

Real
TensorMechanicsPlasticWeakPlaneShear::d2smooth(const RankTwoTensor & stress) const
{
  Real d2smoother2 = 0;
  if (_tip_scheme == "cap")
  {
    Real x = stress(2, 2) - _cap_start;
    Real p = 0;
    Real dp_dx = 0;
    Real d2p_dx2 = 0;
    if (x > 0)
    {
      const Real exp_cap_rate_x = std::exp(-_cap_rate * x);
      p = x * (1.0 - exp_cap_rate_x);
      dp_dx = (1.0 - exp_cap_rate_x) + x * _cap_rate * exp_cap_rate_x;
      d2p_dx2 = 2.0 * _cap_rate * exp_cap_rate_x - x * Utility::pow<2>(_cap_rate) * exp_cap_rate_x;
    }
    d2smoother2 += 2.0 * Utility::pow<2>(dp_dx) + 2.0 * p * d2p_dx2;
  }
  return d2smoother2;
}

void
TensorMechanicsPlasticWeakPlaneShear::activeConstraints(const std::vector<Real> & f,
                                                        const RankTwoTensor & stress,
                                                        Real intnl,
                                                        const RankFourTensor & Eijkl,
                                                        std::vector<bool> & act,
                                                        RankTwoTensor & returned_stress) const
{
  act.assign(1, false);
  returned_stress = stress;

  if (f[0] <= _f_tol)
    return;

  // in the following i will derive returned_stress for the case smoother=0

  Real tanpsi = tan_psi(intnl);
  Real tanphi = tan_phi(intnl);

  // norm is the normal to the yield surface
  // with f having psi (dilation angle) instead of phi:
  // norm(0) = df/dsig(2,0) = df/dsig(0,2)
  // norm(1) = df/dsig(2,1) = df/dsig(1,2)
  // norm(2) = df/dsig(2,2)
  std::vector<Real> norm(3, 0.0);
  const Real tau = std::sqrt(Utility::pow<2>((stress(0, 2) + stress(2, 0)) / 2.0) +
                             Utility::pow<2>((stress(1, 2) + stress(2, 1)) / 2.0));
  if (tau > 0.0)
  {
    norm[0] = 0.25 * (stress(0, 2) + stress(2, 0)) / tau;
    norm[1] = 0.25 * (stress(1, 2) + stress(2, 1)) / tau;
  }
  else
  {
    returned_stress(2, 2) = cohesion(intnl) / tanphi;
    act[0] = true;
    return;
  }
  norm[2] = tanpsi;

  // to get the flow directions, we have to multiply norm by Eijkl.
  // I assume that E(0,2,0,2) = E(1,2,1,2), and E(2,2,0,2) = 0 = E(0,2,1,2), etc
  // with the usual symmetry.  This makes finding the returned_stress
  // much easier.
  // returned_stress = stress - alpha*n
  // where alpha is chosen so that f = 0
  Real alpha = f[0] / (Eijkl(0, 2, 0, 2) + Eijkl(2, 2, 2, 2) * tanpsi * tanphi);

  if (1 - alpha * Eijkl(0, 2, 0, 2) / tau >= 0)
  {
    // returning to the "surface" of the cone
    returned_stress(2, 2) = stress(2, 2) - alpha * Eijkl(2, 2, 2, 2) * norm[2];
    returned_stress(0, 2) = returned_stress(2, 0) =
        stress(0, 2) - alpha * 2.0 * Eijkl(0, 2, 0, 2) * norm[0];
    returned_stress(1, 2) = returned_stress(2, 1) =
        stress(1, 2) - alpha * 2.0 * Eijkl(1, 2, 1, 2) * norm[1];
  }
  else
  {
    // returning to the "tip" of the cone
    returned_stress(2, 2) = cohesion(intnl) / tanphi;
    returned_stress(0, 2) = returned_stress(2, 0) = returned_stress(1, 2) = returned_stress(2, 1) =
        0.0;
  }
  returned_stress(0, 0) =
      stress(0, 0) - Eijkl(0, 0, 2, 2) * (stress(2, 2) - returned_stress(2, 2)) / Eijkl(2, 2, 2, 2);
  returned_stress(1, 1) =
      stress(1, 1) - Eijkl(1, 1, 2, 2) * (stress(2, 2) - returned_stress(2, 2)) / Eijkl(2, 2, 2, 2);

  act[0] = true;
}

std::string
TensorMechanicsPlasticWeakPlaneShear::modelName() const
{
  return "WeakPlaneShear";
}
