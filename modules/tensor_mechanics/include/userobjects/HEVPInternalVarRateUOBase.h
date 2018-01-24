/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEVPINTERNALVARRATEUOBASE_H
#define HEVPINTERNALVARRATEUOBASE_H

#include "DiscreteElementUserObject.h"
#include "RankTwoTensor.h"

class HEVPInternalVarRateUOBase;

template <>
InputParameters validParams<HEVPInternalVarRateUOBase>();

/**
 * This user object is a pure virtual base classs
 * Derived classes computes internal variable rate and derivatives
 */
class HEVPInternalVarRateUOBase : public DiscreteElementUserObject
{
public:
  HEVPInternalVarRateUOBase(const InputParameters & parameters);

  virtual bool computeValue(unsigned int, Real &) const = 0;
  virtual bool computeDerivative(unsigned int, const std::string &, Real &) const = 0;

protected:
  std::string _flow_rate_prop_name;
  const MaterialProperty<Real> & _flow_rate;
};

#endif
