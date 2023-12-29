//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HEVPFlowRateUOBase.h"

/**
 * This user object classs
 * Computes flow rate based on power law and
 * Direction based on J2
 */
class HEVPFlowRatePowerLawJ2 : public HEVPFlowRateUOBase
{
public:
  static InputParameters validParams();

  HEVPFlowRatePowerLawJ2(const InputParameters & parameters);

  virtual bool computeValue(unsigned int, Real &) const;
  virtual bool computeDirection(unsigned int, RankTwoTensor &) const;
  virtual bool computeDerivative(unsigned int, const std::string &, Real &) const;
  virtual bool computeTensorDerivative(unsigned int, const std::string &, RankTwoTensor &) const;

protected:
  Real _ref_flow_rate;
  Real _flow_rate_exponent;
  Real _flow_rate_tol;

  RankTwoTensor computePK2Deviatoric(const RankTwoTensor &, const RankTwoTensor &) const;
  Real computeEqvStress(const RankTwoTensor &, const RankTwoTensor &) const;
};
