/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEVPFLOWRATEPOWERLAWJ2_H
#define HEVPFLOWRATEPOWERLAWJ2_H

#include "HEVPFlowRateUOBase.h"

class HEVPFlowRatePowerLawJ2;

template <>
InputParameters validParams<HEVPFlowRatePowerLawJ2>();

/**
 * This user object classs
 * Computes flow rate based on power law and
 * Direction based on J2
 */
class HEVPFlowRatePowerLawJ2 : public HEVPFlowRateUOBase
{
public:
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

#endif
