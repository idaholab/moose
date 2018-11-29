//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SecondTimeDerivative_H
#define SecondTimeDerivative_H

#include "TimeDerivative.h"

// Forward Declaration
class SecondTimeDerivative;

template <>
InputParameters validParams<SecondTimeDerivative>();

class SecondTimeDerivative : public TimeDerivative
{
public:
  SecondTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Second time derivative of u
  const VariableValue * _u_dotdot;

  /// Derivative of u_dotdot with respect to u
  const VariableValue * _du_dotdot_du;
};

#endif // SecondTimeDerivative_H
