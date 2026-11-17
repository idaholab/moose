//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericIntegratedBC.h"

template <bool is_ad>
class RobinBCTempl : public GenericIntegratedBC<is_ad>
{
public:
  static InputParameters validParams();

  RobinBCTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Value to be applied to the Robin boundary condition
  const Real & _value;
  /// Coefficient to be applied to the Robin boundary condition
  const Real _coef;

  usingGenericIntegratedBCMembers;
};

typedef RobinBCTempl<false> RobinBC;
typedef RobinBCTempl<true> ADRobinBC;
