//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HEVPFlowRatePowerLawJ2.h"

registerMooseObject("TensorMechanicsApp", HEVPFlowRatePowerLawJ2);

InputParameters
HEVPFlowRatePowerLawJ2::validParams()
{
  InputParameters params = HEVPFlowRateUOBase::validParams();
  params.addParam<Real>(
      "reference_flow_rate", 0.001, "Reference flow rate for rate dependent flow");
  params.addParam<Real>("flow_rate_exponent", 10.0, "Power law exponent in flow rate equation");
  params.addParam<Real>("flow_rate_tol", 1e3, "Tolerance for flow rate");
  params.addClassDescription(
      "User object to evaluate power law flow rate and flow direction based on J2");

  return params;
}

HEVPFlowRatePowerLawJ2::HEVPFlowRatePowerLawJ2(const InputParameters & parameters)
  : HEVPFlowRateUOBase(parameters),
    _ref_flow_rate(getParam<Real>("reference_flow_rate")),
    _flow_rate_exponent(getParam<Real>("flow_rate_exponent")),
    _flow_rate_tol(getParam<Real>("flow_rate_tol"))
{
}

bool
HEVPFlowRatePowerLawJ2::computeValue(unsigned int qp, Real & val) const
{
  RankTwoTensor pk2_dev = computePK2Deviatoric(_pk2[qp], _ce[qp]);
  Real eqv_stress = computeEqvStress(pk2_dev, _ce[qp]);
  val = std::pow(eqv_stress / _strength[qp], _flow_rate_exponent) * _ref_flow_rate;

  if (val > _flow_rate_tol)
  {
#ifdef DEBUG
    mooseWarning(
        "Flow rate greater than ", _flow_rate_tol, " ", val, " ", eqv_stress, " ", _strength[qp]);
#endif
    return false;
  }
  return true;
}

bool
HEVPFlowRatePowerLawJ2::computeDirection(unsigned int qp, RankTwoTensor & val) const
{
  RankTwoTensor pk2_dev = computePK2Deviatoric(_pk2[qp], _ce[qp]);
  Real eqv_stress = computeEqvStress(pk2_dev, _ce[qp]);

  val.zero();
  if (eqv_stress > 0.0)
    val = 1.5 / eqv_stress * _ce[qp] * pk2_dev * _ce[qp];

  return true;
}

bool
HEVPFlowRatePowerLawJ2::computeDerivative(unsigned int qp,
                                          const std::string & coupled_var_name,
                                          Real & val) const
{
  val = 0.0;

  if (_strength_prop_name == coupled_var_name)
  {
    RankTwoTensor pk2_dev = computePK2Deviatoric(_pk2[qp], _ce[qp]);
    Real eqv_stress = computeEqvStress(pk2_dev, _ce[qp]);
    val = -_ref_flow_rate * _flow_rate_exponent *
          std::pow(eqv_stress / _strength[qp], _flow_rate_exponent) / _strength[qp];
  }

  return true;
}

bool
HEVPFlowRatePowerLawJ2::computeTensorDerivative(unsigned int qp,
                                                const std::string & coupled_var_name,
                                                RankTwoTensor & val) const
{
  val.zero();

  if (_pk2_prop_name == coupled_var_name)
  {
    RankTwoTensor pk2_dev = computePK2Deviatoric(_pk2[qp], _ce[qp]);
    Real eqv_stress = computeEqvStress(pk2_dev, _ce[qp]);
    Real dflowrate_dseqv = _ref_flow_rate * _flow_rate_exponent *
                           std::pow(eqv_stress / _strength[qp], _flow_rate_exponent - 1.0) /
                           _strength[qp];

    RankTwoTensor tau = pk2_dev * _ce[qp];
    RankTwoTensor dseqv_dpk2dev;

    dseqv_dpk2dev.zero();
    if (eqv_stress > 0.0)
      dseqv_dpk2dev = 1.5 / eqv_stress * tau * _ce[qp];

    RankTwoTensor ce_inv = _ce[qp].inverse();

    RankFourTensor dpk2dev_dpk2;
    for (const auto i : make_range(Moose::dim))
      for (const auto j : make_range(Moose::dim))
        for (const auto k : make_range(Moose::dim))
          for (const auto l : make_range(Moose::dim))
          {
            dpk2dev_dpk2(i, j, k, l) = 0.0;
            if (i == k && j == l)
              dpk2dev_dpk2(i, j, k, l) = 1.0;
            dpk2dev_dpk2(i, j, k, l) -= ce_inv(i, j) * _ce[qp](k, l) / 3.0;
          }
    val = dflowrate_dseqv * dpk2dev_dpk2.transposeMajor() * dseqv_dpk2dev;
  }
  return true;
}

RankTwoTensor
HEVPFlowRatePowerLawJ2::computePK2Deviatoric(const RankTwoTensor & pk2,
                                             const RankTwoTensor & ce) const
{
  return pk2 - (pk2.doubleContraction(ce) * ce.inverse()) / 3.0;
}

Real
HEVPFlowRatePowerLawJ2::computeEqvStress(const RankTwoTensor & pk2_dev,
                                         const RankTwoTensor & ce) const
{
  RankTwoTensor sdev = pk2_dev * ce;
  Real val = sdev.doubleContraction(sdev.transpose());
  return std::sqrt(1.5 * val);
}
