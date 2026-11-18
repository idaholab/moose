//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

#include "AuxKernel.h"

/**
 * Second time derivative of a coupled array variable, to test coupledArrayDotDot()
 */
class ArraySecondTimeDerivativeAux : public ArrayAuxKernel
{
public:
  static InputParameters validParams();

  ArraySecondTimeDerivativeAux(const InputParameters & parameters);

protected:
  virtual RealEigenVector computeValue() override;

  /// Second time derivative of the coupled array variable
  const ArrayVariableValue & _v_dotdot;
};
