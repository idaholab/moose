/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEVPINTERNALVARUOBASE_H
#define HEVPINTERNALVARUOBASE_H

#include "ElementUserObject.h"
#include "RankTwoTensor.h"

class HEVPInternalVarUOBase;

template<>
InputParameters validParams<HEVPInternalVarUOBase>();

/**
 * This user object is a pure virtual base classs
 * Derived classes integrate internal variables
 * Currently only old state is retrieved to use backward Euler
 */
class HEVPInternalVarUOBase : public ElementUserObject
{
public:
  HEVPInternalVarUOBase(const InputParameters & parameters);
  virtual ~HEVPInternalVarUOBase() {}

  void initialize() {}
  void execute() {}
  void finalize() {}
  void threadJoin(const UserObject &) {}

  virtual bool computeValue(unsigned int, Real, Real &) const = 0;
  virtual bool computeDerivative(unsigned int, Real, const std::string &, Real &) const = 0;

protected:
  std::string _intvar_rate_prop_name;
  const MaterialProperty<Real> & _intvar_rate;
  const MaterialProperty<Real> & _this_old;
};

#endif
