//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
