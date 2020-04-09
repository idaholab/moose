//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsPlasticMohrCoulomb.h"
#include "RankFourTensor.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsPlasticMohrCoulomb);

InputParameters
TensorMechanicsPlasticMohrCoulomb::validParams()
{
  InputParameters params = TensorMechanicsPlasticModel::validParams();
  params.addRequiredParam<UserObjectName>(
      "cohesion",
      "A TensorMechanicsHardening UserObject that defines hardening of the cohesion.  "
      "Physically the cohesion should not be negative.");
  params.addRequiredParam<UserObjectName>(
      "friction_angle",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "friction angle (in radians).  Physically the friction angle should be "
      "between 0 and 90deg.");
  params.addRequiredParam<UserObjectName>(
      "dilation_angle",
      "A TensorMechanicsHardening UserObject that defines hardening of the "
      "dilation angle (in radians).  Usually the dilation angle is not greater "
      "than the friction angle, and it is between 0 and 90deg.");
  params.addRangeCheckedParam<Real>(
      "mc_edge_smoother",
      25.0,
      "mc_edge_smoother>=0 & mc_edge_smoother<=30",
      "Smoothing parameter: the edges of the cone are smoothed by the given amount.");
  MooseEnum tip_scheme("hyperbolic cap", "hyperbolic");
  params.addParam<MooseEnum>(
      "tip_scheme", tip_scheme, "Scheme by which the pyramid's tip will be smoothed.");
  params.addRequiredRangeCheckedParam<Real>("mc_tip_smoother",
                                            "mc_tip_smoother>=0",
                                            "Smoothing parameter: the cone vertex at mean = "
                                            "cohesion*cot(friction_angle), will be smoothed by "
                                            "the given amount.  Typical value is 0.1*cohesion");
  params.addParam<Real>(
      "cap_start",
      0.0,
      "For the 'cap' tip_scheme, smoothing is performed in the stress_mean > cap_start region");
  params.addRangeCheckedParam<Real>("cap_rate",
                                    0.0,
                                    "cap_rate>=0",
                                    "For the 'cap' tip_scheme, this controls how quickly the cap "
                                    "degenerates to a hemisphere: small values mean a slow "
                                    "degeneration to a hemisphere (and zero means the 'cap' will "
                                    "be totally inactive).  Typical value is 1/tensile_strength");
  params.addParam<Real>("mc_lode_cutoff",
                        "If the second invariant of stress is less than this "
                        "amount, the Lode angle is assumed to be zero.  This is "
                        "to gaurd against precision-loss problems, and this "
                        "parameter should be set small.  Default = "
                        "0.00001*((yield_Function_tolerance)^2)");
  params.addClassDescription("Non-associative Mohr-Coulomb plasticity with hardening/softening");

  return params;
}

TensorMechanicsPlasticMohrCoulomb::TensorMechanicsPlasticMohrCoulomb(
    const InputParameters & parameters)
  : TensorMechanicsPlasticModel(parameters),
    _cohesion(getUserObject<TensorMechanicsHardeningModel>("cohesion")),
    _phi(getUserObject<TensorMechanicsHardeningModel>("friction_angle")),
    _psi(getUserObject<TensorMechanicsHardeningModel>("dilation_angle")),
    _tip_scheme(getParam<MooseEnum>("tip_scheme")),
    _small_smoother2(Utility::pow<2>(getParam<Real>("mc_tip_smoother"))),
    _cap_start(getParam<Real>("cap_start")),
    _cap_rate(getParam<Real>("cap_rate")),
    _tt(getParam<Real>("mc_edge_smoother") * libMesh::pi / 180.0),
    _costt(std::cos(_tt)),
    _sintt(std::sin(_tt)),
    _cos3tt(std::cos(3 * _tt)),
    _sin3tt(std::sin(3 * _tt)),
    _cos6tt(std::cos(6 * _tt)),
    _sin6tt(std::sin(6 * _tt)),
    _lode_cutoff(parameters.isParamValid("mc_lode_cutoff") ? getParam<Real>("mc_lode_cutoff")
                                                           : 1.0E-5 * Utility::pow<2>(_f_tol))

{
  if (_lode_cutoff < 0)
    mooseError("mc_lode_cutoff must not be negative");

  // With arbitary UserObjects, it is impossible to check everything, and
  // I think this is the best I can do
  if (phi(0) < 0 || psi(0) < 0 || phi(0) > libMesh::pi / 2.0 || psi(0) > libMesh::pi / 2.0)
    mooseError("Mohr-Coulomb friction and dilation angles must lie in [0, Pi/2]");
  if (phi(0) < psi(0))
    mooseError("Mohr-Coulomb friction angle must not be less than Mohr-Coulomb dilation angle");
  if (cohesion(0) < 0)
    mooseError("Mohr-Coulomb cohesion must not be negative");

  // check Abbo et al's convexity constraint (Eqn c.18 in their paper)
  // With an arbitrary UserObject, it is impossible to check for all angles
  // I think the following is the best we can do
  Real sin_angle = std::sin(std::max(phi(0), psi(0)));
  sin_angle = std::max(sin_angle, std::sin(std::max(phi(1E6), psi(1E6))));
  Real rhs = std::sqrt(3) * (35 * std::sin(_tt) + 14 * std::sin(5 * _tt) - 5 * std::sin(7 * _tt)) /
             16 / Utility::pow<5>(std::cos(_tt)) / (11 - 10 * std::cos(2 * _tt));
  if (rhs <= sin_angle)
    mooseError("Mohr-Coulomb edge smoothing angle is too small and a non-convex yield surface will "
               "result.  Please choose a larger value");
}

Real
TensorMechanicsPlasticMohrCoulomb::yieldFunction(const RankTwoTensor & stress, Real intnl) const
{
  Real mean_stress = stress.trace() / 3.0;
  Real sinphi = std::sin(phi(intnl));
  Real cosphi = std::cos(phi(intnl));
  Real sin3Lode = stress.sin3Lode(_lode_cutoff, 0);
  if (std::abs(sin3Lode) <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    stress.symmetricEigenvalues(eigvals);
    return mean_stress * sinphi +
           std::sqrt(smooth(stress) +
                     0.25 * Utility::pow<2>(eigvals[2] - eigvals[0] +
                                            (eigvals[2] + eigvals[0] - 2 * mean_stress) * sinphi)) -
           cohesion(intnl) * cosphi;
  }
  else
  {
    // the edge-smoothed version
    Real aaa, bbb, ccc;
    abbo(sin3Lode, sinphi, aaa, bbb, ccc);
    Real kk = aaa + bbb * sin3Lode + ccc * Utility::pow<2>(sin3Lode);
    Real sibar2 = stress.secondInvariant();
    return mean_stress * sinphi + std::sqrt(smooth(stress) + sibar2 * Utility::pow<2>(kk)) -
           cohesion(intnl) * cosphi;
  }
}

RankTwoTensor
TensorMechanicsPlasticMohrCoulomb::df_dsig(const RankTwoTensor & stress, const Real sin_angle) const
{
  Real mean_stress = stress.trace() / 3.0;
  RankTwoTensor dmean_stress = stress.dtrace() / 3.0;
  Real sin3Lode = stress.sin3Lode(_lode_cutoff, 0);
  if (std::abs(sin3Lode) <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    std::vector<RankTwoTensor> deigvals;
    stress.dsymmetricEigenvalues(eigvals, deigvals);
    Real tmp = eigvals[2] - eigvals[0] + (eigvals[2] + eigvals[0] - 2.0 * mean_stress) * sin_angle;
    RankTwoTensor dtmp =
        deigvals[2] - deigvals[0] + (deigvals[2] + deigvals[0] - 2.0 * dmean_stress) * sin_angle;
    Real denom = std::sqrt(smooth(stress) + 0.25 * Utility::pow<2>(tmp));
    return dmean_stress * sin_angle +
           (0.5 * dsmooth(stress) * dmean_stress + 0.25 * tmp * dtmp) / denom;
  }
  else
  {
    // the edge-smoothed version
    Real aaa, bbb, ccc;
    abbo(sin3Lode, sin_angle, aaa, bbb, ccc);
    Real kk = aaa + bbb * sin3Lode + ccc * Utility::pow<2>(sin3Lode);
    RankTwoTensor dkk = (bbb + 2 * ccc * sin3Lode) * stress.dsin3Lode(_lode_cutoff);
    Real sibar2 = stress.secondInvariant();
    RankTwoTensor dsibar2 = stress.dsecondInvariant();
    Real denom = std::sqrt(smooth(stress) + sibar2 * Utility::pow<2>(kk));
    return dmean_stress * sin_angle + (0.5 * dsmooth(stress) * dmean_stress +
                                       0.5 * dsibar2 * Utility::pow<2>(kk) + sibar2 * kk * dkk) /
                                          denom;
  }
}

RankTwoTensor
TensorMechanicsPlasticMohrCoulomb::dyieldFunction_dstress(const RankTwoTensor & stress,
                                                          Real intnl) const
{
  Real sinphi = std::sin(phi(intnl));
  return df_dsig(stress, sinphi);
}

Real
TensorMechanicsPlasticMohrCoulomb::dyieldFunction_dintnl(const RankTwoTensor & stress,
                                                         Real intnl) const
{
  Real sin_angle = std::sin(phi(intnl));
  Real cos_angle = std::cos(phi(intnl));
  Real dsin_angle = cos_angle * dphi(intnl);
  Real dcos_angle = -sin_angle * dphi(intnl);

  Real mean_stress = stress.trace() / 3.0;
  Real sin3Lode = stress.sin3Lode(_lode_cutoff, 0);
  if (std::abs(sin3Lode) <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    stress.symmetricEigenvalues(eigvals);
    Real tmp = eigvals[2] - eigvals[0] + (eigvals[2] + eigvals[0] - 2 * mean_stress) * sin_angle;
    Real dtmp = (eigvals[2] + eigvals[0] - 2 * mean_stress) * dsin_angle;
    Real denom = std::sqrt(smooth(stress) + 0.25 * Utility::pow<2>(tmp));
    return mean_stress * dsin_angle + 0.25 * tmp * dtmp / denom - dcohesion(intnl) * cos_angle -
           cohesion(intnl) * dcos_angle;
  }
  else
  {
    // the edge-smoothed version
    Real aaa, bbb, ccc;
    abbo(sin3Lode, sin_angle, aaa, bbb, ccc);
    Real daaa, dbbb, dccc;
    dabbo(sin3Lode, sin_angle, daaa, dbbb, dccc);
    Real kk = aaa + bbb * sin3Lode + ccc * Utility::pow<2>(sin3Lode);
    Real dkk = (daaa + dbbb * sin3Lode + dccc * Utility::pow<2>(sin3Lode)) * dsin_angle;
    Real sibar2 = stress.secondInvariant();
    Real denom = std::sqrt(smooth(stress) + sibar2 * Utility::pow<2>(kk));
    return mean_stress * dsin_angle + sibar2 * kk * dkk / denom - dcohesion(intnl) * cos_angle -
           cohesion(intnl) * dcos_angle;
  }
}

RankTwoTensor
TensorMechanicsPlasticMohrCoulomb::flowPotential(const RankTwoTensor & stress, Real intnl) const
{
  Real sinpsi = std::sin(psi(intnl));
  return df_dsig(stress, sinpsi);
}

RankFourTensor
TensorMechanicsPlasticMohrCoulomb::dflowPotential_dstress(const RankTwoTensor & stress,
                                                          Real intnl) const
{
  RankFourTensor dr_dstress;
  Real sin_angle = std::sin(psi(intnl));
  Real mean_stress = stress.trace() / 3.0;
  RankTwoTensor dmean_stress = stress.dtrace() / 3.0;
  Real sin3Lode = stress.sin3Lode(_lode_cutoff, 0);
  if (std::abs(sin3Lode) <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    std::vector<RankTwoTensor> deigvals;
    std::vector<RankFourTensor> d2eigvals;
    stress.dsymmetricEigenvalues(eigvals, deigvals);
    stress.d2symmetricEigenvalues(d2eigvals);

    Real tmp = eigvals[2] - eigvals[0] + (eigvals[2] + eigvals[0] - 2.0 * mean_stress) * sin_angle;
    RankTwoTensor dtmp =
        deigvals[2] - deigvals[0] + (deigvals[2] + deigvals[0] - 2.0 * dmean_stress) * sin_angle;
    Real denom = std::sqrt(smooth(stress) + 0.25 * Utility::pow<2>(tmp));
    Real denom3 = Utility::pow<3>(denom);
    Real d2smooth_over_denom = d2smooth(stress) / denom;
    RankTwoTensor numer = dsmooth(stress) * dmean_stress + 0.5 * tmp * dtmp;

    dr_dstress = 0.25 * tmp *
                 (d2eigvals[2] - d2eigvals[0] + (d2eigvals[2] + d2eigvals[0]) * sin_angle) / denom;

    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j < 3; ++j)
        for (unsigned k = 0; k < 3; ++k)
          for (unsigned l = 0; l < 3; ++l)
          {
            dr_dstress(i, j, k, l) +=
                0.5 * d2smooth_over_denom * dmean_stress(i, j) * dmean_stress(k, l);
            dr_dstress(i, j, k, l) += 0.25 * dtmp(i, j) * dtmp(k, l) / denom;
            dr_dstress(i, j, k, l) -= 0.25 * numer(i, j) * numer(k, l) / denom3;
          }
  }
  else
  {
    // the edge-smoothed version
    Real aaa, bbb, ccc;
    abbo(sin3Lode, sin_angle, aaa, bbb, ccc);
    RankTwoTensor dsin3Lode = stress.dsin3Lode(_lode_cutoff);
    Real kk = aaa + bbb * sin3Lode + ccc * Utility::pow<2>(sin3Lode);
    RankTwoTensor dkk = (bbb + 2 * ccc * sin3Lode) * dsin3Lode;
    RankFourTensor d2kk = (bbb + 2 * ccc * sin3Lode) * stress.d2sin3Lode(_lode_cutoff);
    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j < 3; ++j)
        for (unsigned k = 0; k < 3; ++k)
          for (unsigned l = 0; l < 3; ++l)
            d2kk(i, j, k, l) += 2 * ccc * dsin3Lode(i, j) * dsin3Lode(k, l);

    Real sibar2 = stress.secondInvariant();
    RankTwoTensor dsibar2 = stress.dsecondInvariant();
    RankFourTensor d2sibar2 = stress.d2secondInvariant();

    Real denom = std::sqrt(smooth(stress) + sibar2 * Utility::pow<2>(kk));
    Real denom3 = Utility::pow<3>(denom);
    Real d2smooth_over_denom = d2smooth(stress) / denom;
    RankTwoTensor numer_full =
        0.5 * dsmooth(stress) * dmean_stress + 0.5 * dsibar2 * kk * kk + sibar2 * kk * dkk;

    dr_dstress = (0.5 * d2sibar2 * Utility::pow<2>(kk) + sibar2 * kk * d2kk) / denom;
    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j < 3; ++j)
        for (unsigned k = 0; k < 3; ++k)
          for (unsigned l = 0; l < 3; ++l)
          {
            dr_dstress(i, j, k, l) +=
                0.5 * d2smooth_over_denom * dmean_stress(i, j) * dmean_stress(k, l);
            dr_dstress(i, j, k, l) +=
                (dsibar2(i, j) * dkk(k, l) * kk + dkk(i, j) * dsibar2(k, l) * kk +
                 sibar2 * dkk(i, j) * dkk(k, l)) /
                denom;
            dr_dstress(i, j, k, l) -= numer_full(i, j) * numer_full(k, l) / denom3;
          }
  }
  return dr_dstress;
}

RankTwoTensor
TensorMechanicsPlasticMohrCoulomb::dflowPotential_dintnl(const RankTwoTensor & stress,
                                                         Real intnl) const
{
  Real sin_angle = std::sin(psi(intnl));
  Real dsin_angle = std::cos(psi(intnl)) * dpsi(intnl);

  Real mean_stress = stress.trace() / 3.0;
  RankTwoTensor dmean_stress = stress.dtrace() / 3.0;
  Real sin3Lode = stress.sin3Lode(_lode_cutoff, 0);

  if (std::abs(sin3Lode) <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    std::vector<RankTwoTensor> deigvals;
    stress.dsymmetricEigenvalues(eigvals, deigvals);
    Real tmp = eigvals[2] - eigvals[0] + (eigvals[2] + eigvals[0] - 2.0 * mean_stress) * sin_angle;
    Real dtmp_dintnl = (eigvals[2] + eigvals[0] - 2 * mean_stress) * dsin_angle;
    RankTwoTensor dtmp_dstress =
        deigvals[2] - deigvals[0] + (deigvals[2] + deigvals[0] - 2.0 * dmean_stress) * sin_angle;
    RankTwoTensor d2tmp_dstress_dintnl =
        (deigvals[2] + deigvals[0] - 2.0 * dmean_stress) * dsin_angle;
    Real denom = std::sqrt(smooth(stress) + 0.25 * Utility::pow<2>(tmp));
    return dmean_stress * dsin_angle + 0.25 * dtmp_dintnl * dtmp_dstress / denom +
           0.25 * tmp * d2tmp_dstress_dintnl / denom -
           0.5 * (dsmooth(stress) * dmean_stress + 0.5 * tmp * dtmp_dstress) * 0.25 * tmp *
               dtmp_dintnl / Utility::pow<3>(denom);
  }
  else
  {
    // the edge-smoothed version
    Real aaa, bbb, ccc;
    abbo(sin3Lode, sin_angle, aaa, bbb, ccc);
    Real kk = aaa + bbb * sin3Lode + ccc * Utility::pow<2>(sin3Lode);

    Real daaa, dbbb, dccc;
    dabbo(sin3Lode, sin_angle, daaa, dbbb, dccc);
    Real dkk_dintnl = (daaa + dbbb * sin3Lode + dccc * Utility::pow<2>(sin3Lode)) * dsin_angle;
    RankTwoTensor dkk_dstress = (bbb + 2 * ccc * sin3Lode) * stress.dsin3Lode(_lode_cutoff);
    RankTwoTensor d2kk_dstress_dintnl =
        (dbbb + 2 * dccc * sin3Lode) * stress.dsin3Lode(_lode_cutoff) * dsin_angle;

    Real sibar2 = stress.secondInvariant();
    RankTwoTensor dsibar2 = stress.dsecondInvariant();
    Real denom = std::sqrt(smooth(stress) + sibar2 * Utility::pow<2>(kk));

    return dmean_stress * dsin_angle +
           (dsibar2 * kk * dkk_dintnl + sibar2 * dkk_dintnl * dkk_dstress +
            sibar2 * kk * d2kk_dstress_dintnl) /
               denom -
           (0.5 * dsmooth(stress) * dmean_stress + 0.5 * dsibar2 * Utility::pow<2>(kk) +
            sibar2 * kk * dkk_dstress) *
               sibar2 * kk * dkk_dintnl / Utility::pow<3>(denom);
  }
}

Real
TensorMechanicsPlasticMohrCoulomb::cohesion(const Real internal_param) const
{
  return _cohesion.value(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulomb::dcohesion(const Real internal_param) const
{
  return _cohesion.derivative(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulomb::phi(const Real internal_param) const
{
  return _phi.value(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulomb::dphi(const Real internal_param) const
{
  return _phi.derivative(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulomb::psi(const Real internal_param) const
{
  return _psi.value(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulomb::dpsi(const Real internal_param) const
{
  return _psi.derivative(internal_param);
}

void
TensorMechanicsPlasticMohrCoulomb::abbo(
    const Real sin3lode, const Real sin_angle, Real & aaa, Real & bbb, Real & ccc) const
{
  Real tmp1 = (sin3lode >= 0 ? _costt - sin_angle * _sintt / std::sqrt(3.0)
                             : _costt + sin_angle * _sintt / std::sqrt(3.0));
  Real tmp2 = (sin3lode >= 0 ? _sintt + sin_angle * _costt / std::sqrt(3.0)
                             : -_sintt + sin_angle * _costt / std::sqrt(3.0));

  ccc = -_cos3tt * tmp1;
  ccc += (sin3lode >= 0 ? -3 * _sin3tt * tmp2 : 3 * _sin3tt * tmp2);
  ccc /= 18 * Utility::pow<3>(_cos3tt);

  bbb = (sin3lode >= 0 ? _sin6tt * tmp1 : -_sin6tt * tmp1);
  bbb -= 6 * _cos6tt * tmp2;
  bbb /= 18 * Utility::pow<3>(_cos3tt);

  aaa = (sin3lode >= 0 ? -sin_angle * _sintt / std::sqrt(3.0) - bbb * _sin3tt
                       : sin_angle * _sintt / std::sqrt(3.0) + bbb * _sin3tt);
  aaa += -ccc * Utility::pow<2>(_sin3tt) + _costt;
}

void
TensorMechanicsPlasticMohrCoulomb::dabbo(
    const Real sin3lode, const Real /*sin_angle*/, Real & daaa, Real & dbbb, Real & dccc) const
{
  Real dtmp1 = (sin3lode >= 0 ? -_sintt / std::sqrt(3.0) : _sintt / std::sqrt(3.0));
  Real dtmp2 = _costt / std::sqrt(3.0);

  dccc = -_cos3tt * dtmp1;
  dccc += (sin3lode >= 0 ? -3 * _sin3tt * dtmp2 : 3 * _sin3tt * dtmp2);
  dccc /= 18 * Utility::pow<3>(_cos3tt);

  dbbb = (sin3lode >= 0 ? _sin6tt * dtmp1 : -_sin6tt * dtmp1);
  dbbb -= 6 * _cos6tt * dtmp2;
  dbbb /= 18 * Utility::pow<3>(_cos3tt);

  daaa = (sin3lode >= 0 ? -_sintt / std::sqrt(3.0) - dbbb * _sin3tt
                        : _sintt / std::sqrt(3.0) + dbbb * _sin3tt);
  daaa += -dccc * Utility::pow<2>(_sin3tt);
}

Real
TensorMechanicsPlasticMohrCoulomb::smooth(const RankTwoTensor & stress) const
{
  Real smoother2 = _small_smoother2;
  if (_tip_scheme == "cap")
  {
    Real x = stress.trace() / 3.0 - _cap_start;
    Real p = 0;
    if (x > 0)
      p = x * (1 - std::exp(-_cap_rate * x));
    smoother2 += Utility::pow<2>(p);
  }
  return smoother2;
}

Real
TensorMechanicsPlasticMohrCoulomb::dsmooth(const RankTwoTensor & stress) const
{
  Real dsmoother2 = 0;
  if (_tip_scheme == "cap")
  {
    Real x = stress.trace() / 3.0 - _cap_start;
    Real p = 0;
    Real dp_dx = 0;
    if (x > 0)
    {
      p = x * (1 - std::exp(-_cap_rate * x));
      dp_dx = (1 - std::exp(-_cap_rate * x)) + x * _cap_rate * std::exp(-_cap_rate * x);
    }
    dsmoother2 += 2 * p * dp_dx;
  }
  return dsmoother2;
}

Real
TensorMechanicsPlasticMohrCoulomb::d2smooth(const RankTwoTensor & stress) const
{
  Real d2smoother2 = 0;
  if (_tip_scheme == "cap")
  {
    Real x = stress.trace() / 3.0 - _cap_start;
    Real p = 0;
    Real dp_dx = 0;
    Real d2p_dx2 = 0;
    if (x > 0)
    {
      p = x * (1 - std::exp(-_cap_rate * x));
      dp_dx = (1 - std::exp(-_cap_rate * x)) + x * _cap_rate * std::exp(-_cap_rate * x);
      d2p_dx2 = 2 * _cap_rate * std::exp(-_cap_rate * x) -
                x * Utility::pow<2>(_cap_rate) * std::exp(-_cap_rate * x);
    }
    d2smoother2 += 2 * Utility::pow<2>(dp_dx) + 2 * p * d2p_dx2;
  }
  return d2smoother2;
}

std::string
TensorMechanicsPlasticMohrCoulomb::modelName() const
{
  return "MohrCoulomb";
}
