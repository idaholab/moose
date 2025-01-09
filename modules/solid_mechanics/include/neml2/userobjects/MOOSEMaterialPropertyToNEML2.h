//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MOOSEToNEML2Batched.h"

#include "RankTwoTensor.h"
#include "SymmetricRankTwoTensor.h"

/**
 * Gather a MOOSE material property for insertion into the NEML2 model.
 */
template <typename T, unsigned int state>
class MOOSEMaterialPropertyToNEML2 : public MOOSEToNEML2Batched<T>
{
public:
  static InputParameters validParams();

  MOOSEMaterialPropertyToNEML2(const InputParameters & params);

#ifdef NEML2_ENABLED
protected:
  const MooseArray<T> & elemMOOSEData() const override { return _mat_prop.get(); }

  /// MOOSE material property to read data from
  const MaterialProperty<T> & _mat_prop;
#endif
};

#define DefineMOOSEMaterialPropertyToNEML2Alias(T, alias)                                          \
  using MOOSE##alias##MaterialPropertyToNEML2 = MOOSEMaterialPropertyToNEML2<T, 0>;                \
  using MOOSEOld##alias##MaterialPropertyToNEML2 = MOOSEMaterialPropertyToNEML2<T, 1>

DefineMOOSEMaterialPropertyToNEML2Alias(Real, Real);
DefineMOOSEMaterialPropertyToNEML2Alias(RankTwoTensor, RankTwoTensor);
DefineMOOSEMaterialPropertyToNEML2Alias(SymmetricRankTwoTensor, SymmetricRankTwoTensor);
DefineMOOSEMaterialPropertyToNEML2Alias(RealVectorValue, RealVectorValue);
