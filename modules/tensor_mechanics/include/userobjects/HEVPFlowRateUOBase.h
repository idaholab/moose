/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEVPFLOWRATEUOBASE_H
#define HEVPFLOWRATEUOBASE_H

#include "DiscreteElementUserObject.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

class HEVPFlowRateUOBase;

template <>
InputParameters validParams<HEVPFlowRateUOBase>();

/**
 * This user object is a pure virtual base classs
 * Derived classes computes flow rate, direction and derivatives
 */
class HEVPFlowRateUOBase : public DiscreteElementUserObject
{
public:
  HEVPFlowRateUOBase(const InputParameters & parameters);

  virtual bool computeValue(unsigned int, Real &) const = 0;
  virtual bool computeDirection(unsigned int, RankTwoTensor &) const = 0;
  virtual bool computeDerivative(unsigned int, const std::string &, Real &) const = 0;
  virtual bool
  computeTensorDerivative(unsigned int, const std::string &, RankTwoTensor &) const = 0;

protected:
  std::string _strength_prop_name;
  std::string _base_name;
  const MaterialProperty<Real> & _strength;
  std::string _pk2_prop_name;
  const MaterialProperty<RankTwoTensor> & _pk2;
  const MaterialProperty<RankTwoTensor> & _ce;
};

#endif
