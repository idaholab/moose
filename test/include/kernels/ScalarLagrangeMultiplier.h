//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SCALARLAGRANGEMULTIPLIER_H
#define SCALARLAGRANGEMULTIPLIER_H

#include "Kernel.h"

class ScalarLagrangeMultiplier;

template <>
InputParameters validParams<ScalarLagrangeMultiplier>();

/**
 * Implements coupling of Lagrange multiplier (as a scalar variable) to a simple constraint of type
 * (g(u) - c = 0)
 */
class ScalarLagrangeMultiplier : public Kernel
{
public:
  ScalarLagrangeMultiplier(const InputParameters & parameters);
  virtual ~ScalarLagrangeMultiplier();

  virtual void computeOffDiagJacobianScalar(unsigned int jvar);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _lambda_var;
  VariableValue & _lambda;
};

#endif /* SCALARLAGRANGEMULTIPLIER_H */
