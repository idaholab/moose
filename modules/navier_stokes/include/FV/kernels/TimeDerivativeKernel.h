//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CNSKernel.h"

class TimeDerivativeKernel;

declareADValidParams(TimeDerivativeKernel);

/**
 * A base class kernel representing the time derivative component of the system of
 * coupled equations. Derived classes specify the form of the time derivative.
 */
class TimeDerivativeKernel : public CNSKernel
{
public:
  TimeDerivativeKernel(const InputParameters & parameters);

protected:
  virtual ADReal weakResidual() override final;

  virtual ADReal strongResidual() override final;

  /// compute time derivative of the equation
  virtual ADReal timeDerivative() = 0;

  /// porosity
  const VariableValue & _eps;

};
