//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDFORCE_H
#define COUPLEDFORCE_H

#include "Kernel.h"

// Forward Declaration
class CoupledForce;

template <>
InputParameters validParams<CoupledForce>();

/**
 * Simple class to demonstrate off diagonal Jacobian contributions.
 */
class CoupledForce : public Kernel
{
public:
  CoupledForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;

  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

private:
  unsigned int _v_var;
  const VariableValue & _v;
  Real _coef;
};

#endif // COUPLEDFORCE_H
