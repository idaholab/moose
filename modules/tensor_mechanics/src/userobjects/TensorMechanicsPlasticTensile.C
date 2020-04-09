//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsPlasticTensile.h"
#include "RankFourTensor.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsPlasticTensile);

InputParameters
TensorMechanicsPlasticTensile::validParams()
{
  InputParameters params = TensorMechanicsPlasticModel::validParams();
  params.addRequiredParam<UserObjectName>(
      "tensile_strength",
      "A TensorMechanicsHardening UserObject that defines hardening of the tensile strength");
  params.addRangeCheckedParam<Real>(
      "tensile_edge_smoother",
      25.0,
      "tensile_edge_smoother>=0 & tensile_edge_smoother<=30",
      "Smoothing parameter: the edges of the cone are smoothed by the given amount.");
  MooseEnum tip_scheme("hyperbolic cap", "hyperbolic");
  params.addParam<MooseEnum>(
      "tip_scheme", tip_scheme, "Scheme by which the pyramid's tip will be smoothed.");
  params.addRequiredRangeCheckedParam<Real>("tensile_tip_smoother",
                                            "tensile_tip_smoother>=0",
                                            "For the 'hyperbolic' tip_scheme, the pyramid vertex "
                                            "will be smoothed by the given amount.  For the 'cap' "
                                            "tip_scheme, additional smoothing will occur.  Typical "
                                            "value is 0.1*tensile_strength");
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
  params.addParam<Real>("tensile_lode_cutoff",
                        "If the second invariant of stress is less than "
                        "this amount, the Lode angle is assumed to be zero. "
                        "This is to guard against precision-loss problems, "
                        "and this parameter should be set small.  Default = "
                        "0.00001*((yield_Function_tolerance)^2)");
  params.addClassDescription(
      "Associative tensile plasticity with hardening/softening, and tensile_strength = 1");

  return params;
}

TensorMechanicsPlasticTensile::TensorMechanicsPlasticTensile(const InputParameters & parameters)
  : TensorMechanicsPlasticModel(parameters),
    _strength(getUserObject<TensorMechanicsHardeningModel>("tensile_strength")),
    _tip_scheme(getParam<MooseEnum>("tip_scheme")),
    _small_smoother2(Utility::pow<2>(getParam<Real>("tensile_tip_smoother"))),
    _cap_start(getParam<Real>("cap_start")),
    _cap_rate(getParam<Real>("cap_rate")),
    _tt(getParam<Real>("tensile_edge_smoother") * libMesh::pi / 180.0),
    _sin3tt(std::sin(3.0 * _tt)),
    _lode_cutoff(parameters.isParamValid("tensile_lode_cutoff")
                     ? getParam<Real>("tensile_lode_cutoff")
                     : 1.0e-5 * Utility::pow<2>(_f_tol))

{
  if (_lode_cutoff < 0)
    mooseError("tensile_lode_cutoff must not be negative");
  _ccc = (-std::cos(3.0 * _tt) * (std::cos(_tt) - std::sin(_tt) / std::sqrt(3.0)) -
          3.0 * _sin3tt * (std::sin(_tt) + std::cos(_tt) / std::sqrt(3.0))) /
         (18.0 * Utility::pow<3>(std::cos(3.0 * _tt)));
  _bbb = (std::sin(6.0 * _tt) * (std::cos(_tt) - std::sin(_tt) / std::sqrt(3.0)) -
          6.0 * std::cos(6.0 * _tt) * (std::sin(_tt) + std::cos(_tt) / std::sqrt(3.0))) /
         (18.0 * Utility::pow<3>(std::cos(3.0 * _tt)));
  _aaa = -std::sin(_tt) / std::sqrt(3.0) - _bbb * _sin3tt - _ccc * Utility::pow<2>(_sin3tt) +
         std::cos(_tt);
}

Real
TensorMechanicsPlasticTensile::yieldFunction(const RankTwoTensor & stress, Real intnl) const
{
  Real mean_stress = stress.trace() / 3.0;
  Real sin3Lode = stress.sin3Lode(_lode_cutoff, 0);
  if (sin3Lode <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    stress.symmetricEigenvalues(eigvals);
    return mean_stress + std::sqrt(smooth(stress) + Utility::pow<2>(eigvals[2] - mean_stress)) -
           tensile_strength(intnl);
  }
  else
  {
    // the edge-smoothed version
    Real kk = _aaa + _bbb * sin3Lode + _ccc * Utility::pow<2>(sin3Lode);
    Real sibar2 = stress.secondInvariant();
    return mean_stress + std::sqrt(smooth(stress) + sibar2 * Utility::pow<2>(kk)) -
           tensile_strength(intnl);
  }
}

RankTwoTensor
TensorMechanicsPlasticTensile::dyieldFunction_dstress(const RankTwoTensor & stress,
                                                      Real /*intnl*/) const
{
  Real mean_stress = stress.trace() / 3.0;
  RankTwoTensor dmean_stress = stress.dtrace() / 3.0;
  Real sin3Lode = stress.sin3Lode(_lode_cutoff, 0);
  if (sin3Lode <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    std::vector<RankTwoTensor> deigvals;
    stress.dsymmetricEigenvalues(eigvals, deigvals);
    Real denom = std::sqrt(smooth(stress) + Utility::pow<2>(eigvals[2] - mean_stress));
    return dmean_stress + (0.5 * dsmooth(stress) * dmean_stress +
                           (eigvals[2] - mean_stress) * (deigvals[2] - dmean_stress)) /
                              denom;
  }
  else
  {
    // the edge-smoothed version
    Real kk = _aaa + _bbb * sin3Lode + _ccc * Utility::pow<2>(sin3Lode);
    RankTwoTensor dkk = (_bbb + 2.0 * _ccc * sin3Lode) * stress.dsin3Lode(_lode_cutoff);
    Real sibar2 = stress.secondInvariant();
    RankTwoTensor dsibar2 = stress.dsecondInvariant();
    Real denom = std::sqrt(smooth(stress) + sibar2 * Utility::pow<2>(kk));
    return dmean_stress + (0.5 * dsmooth(stress) * dmean_stress +
                           0.5 * dsibar2 * Utility::pow<2>(kk) + sibar2 * kk * dkk) /
                              denom;
  }
}

Real
TensorMechanicsPlasticTensile::dyieldFunction_dintnl(const RankTwoTensor & /*stress*/,
                                                     Real intnl) const
{
  return -dtensile_strength(intnl);
}

RankTwoTensor
TensorMechanicsPlasticTensile::flowPotential(const RankTwoTensor & stress, Real intnl) const
{
  // This plasticity is associative so
  return dyieldFunction_dstress(stress, intnl);
}

RankFourTensor
TensorMechanicsPlasticTensile::dflowPotential_dstress(const RankTwoTensor & stress,
                                                      Real /*intnl*/) const
{
  Real mean_stress = stress.trace() / 3.0;
  RankTwoTensor dmean_stress = stress.dtrace() / 3.0;
  Real sin3Lode = stress.sin3Lode(_lode_cutoff, 0);
  if (sin3Lode <= _sin3tt)
  {
    // the non-edge-smoothed version
    std::vector<Real> eigvals;
    std::vector<RankTwoTensor> deigvals;
    std::vector<RankFourTensor> d2eigvals;
    stress.dsymmetricEigenvalues(eigvals, deigvals);
    stress.d2symmetricEigenvalues(d2eigvals);

    Real denom = std::sqrt(smooth(stress) + Utility::pow<2>(eigvals[2] - mean_stress));
    Real denom3 = Utility::pow<3>(denom);
    RankTwoTensor numer_part = deigvals[2] - dmean_stress;
    RankTwoTensor numer_full =
        0.5 * dsmooth(stress) * dmean_stress + (eigvals[2] - mean_stress) * numer_part;
    Real d2smooth_over_denom = d2smooth(stress) / denom;

    RankFourTensor dr_dstress = (eigvals[2] - mean_stress) * d2eigvals[2] / denom;
    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j < 3; ++j)
        for (unsigned k = 0; k < 3; ++k)
          for (unsigned l = 0; l < 3; ++l)
          {
            dr_dstress(i, j, k, l) +=
                0.5 * d2smooth_over_denom * dmean_stress(i, j) * dmean_stress(k, l);
            dr_dstress(i, j, k, l) += numer_part(i, j) * numer_part(k, l) / denom;
            dr_dstress(i, j, k, l) -= numer_full(i, j) * numer_full(k, l) / denom3;
          }
    return dr_dstress;
  }
  else
  {
    // the edge-smoothed version
    RankTwoTensor dsin3Lode = stress.dsin3Lode(_lode_cutoff);
    Real kk = _aaa + _bbb * sin3Lode + _ccc * Utility::pow<2>(sin3Lode);
    RankTwoTensor dkk = (_bbb + 2.0 * _ccc * sin3Lode) * dsin3Lode;
    RankFourTensor d2kk = (_bbb + 2.0 * _ccc * sin3Lode) * stress.d2sin3Lode(_lode_cutoff);
    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j < 3; ++j)
        for (unsigned k = 0; k < 3; ++k)
          for (unsigned l = 0; l < 3; ++l)
            d2kk(i, j, k, l) += 2.0 * _ccc * dsin3Lode(i, j) * dsin3Lode(k, l);

    Real sibar2 = stress.secondInvariant();
    RankTwoTensor dsibar2 = stress.dsecondInvariant();
    RankFourTensor d2sibar2 = stress.d2secondInvariant();

    Real denom = std::sqrt(smooth(stress) + sibar2 * Utility::pow<2>(kk));
    Real denom3 = Utility::pow<3>(denom);
    Real d2smooth_over_denom = d2smooth(stress) / denom;
    RankTwoTensor numer_full =
        0.5 * dsmooth(stress) * dmean_stress + 0.5 * dsibar2 * kk * kk + sibar2 * kk * dkk;

    RankFourTensor dr_dstress = (0.5 * d2sibar2 * Utility::pow<2>(kk) + sibar2 * kk * d2kk) / denom;
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
    return dr_dstress;
  }
}

RankTwoTensor
TensorMechanicsPlasticTensile::dflowPotential_dintnl(const RankTwoTensor & /*stress*/,
                                                     Real /*intnl*/) const
{
  return RankTwoTensor();
}

Real
TensorMechanicsPlasticTensile::tensile_strength(const Real internal_param) const
{
  return _strength.value(internal_param);
}

Real
TensorMechanicsPlasticTensile::dtensile_strength(const Real internal_param) const
{
  return _strength.derivative(internal_param);
}

Real
TensorMechanicsPlasticTensile::smooth(const RankTwoTensor & stress) const
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
TensorMechanicsPlasticTensile::dsmooth(const RankTwoTensor & stress) const
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
    dsmoother2 += 2.0 * p * dp_dx;
  }
  return dsmoother2;
}

Real
TensorMechanicsPlasticTensile::d2smooth(const RankTwoTensor & stress) const
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
      d2p_dx2 = 2.0 * _cap_rate * std::exp(-_cap_rate * x) -
                x * Utility::pow<2>(_cap_rate) * std::exp(-_cap_rate * x);
    }
    d2smoother2 += 2.0 * Utility::pow<2>(dp_dx) + 2.0 * p * d2p_dx2;
  }
  return d2smoother2;
}

std::string
TensorMechanicsPlasticTensile::modelName() const
{
  return "Tensile";
}
