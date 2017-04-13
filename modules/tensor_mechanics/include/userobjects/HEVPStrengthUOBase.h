/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEVPSTRENGTHUOBASE_H
#define HEVPSTRENGTHUOBASE_H

#include "DiscreteElementUserObject.h"
#include "RankTwoTensor.h"

class HEVPStrengthUOBase;

template <>
InputParameters validParams<HEVPStrengthUOBase>();

/**
 * This user object is a pure virtual base classs
 * Derived classes computes material resistances and derivatives
 */
class HEVPStrengthUOBase : public DiscreteElementUserObject
{
public:
  HEVPStrengthUOBase(const InputParameters & parameters);

  virtual bool computeValue(unsigned int, Real &) const = 0;
  virtual bool computeDerivative(unsigned int, const std::string &, Real &) const = 0;

protected:
  std::string _intvar_prop_name;
  const MaterialProperty<Real> & _intvar;
};

#endif
