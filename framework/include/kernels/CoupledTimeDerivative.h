//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "VectorKernel.h"

template <typename K>
struct DotType;

template <>
struct DotType<Kernel>
{
  typedef VariableValue type;
};

template <>
struct DotType<VectorKernel>
{
  typedef VectorVariableValue type;
};

/**
 * This calculates the time derivative for a coupled variable
 **/
template <typename K>
class CoupledTimeDerivativeTempl : public K
{
public:
  static InputParameters validParams();

  CoupledTimeDerivativeTempl(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const typename DotType<K>::type & _v_dot;
  const VariableValue & _dv_dot;
  const unsigned int _v_var;

  Real _coeff;
};

using CoupledTimeDerivative = CoupledTimeDerivativeTempl<Kernel>;
using VectorCoupledTimeDerivative = CoupledTimeDerivativeTempl<VectorKernel>;
