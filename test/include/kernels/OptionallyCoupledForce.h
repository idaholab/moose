//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef OPTIONALLYCOUPLEDFORCE_H
#define OPTIONALLYCOUPLEDFORCE_H

#include "Kernel.h"

// Forward Declaration
class OptionallyCoupledForce;

template <>
InputParameters validParams<OptionallyCoupledForce>();

/**
 * Simple class to demonstrate off diagonal Jacobian contributions.
 */
class OptionallyCoupledForce : public Kernel
{
public:
  OptionallyCoupledForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  unsigned int _v_var;
  const VariableValue & _v;
  const VariableGradient & _grad_v;
  const VariableSecond & _second_v;
  const VariableValue & _v_dot;
  const VariableValue & _v_dot_du;
  bool _v_coupled;
};

#endif // OPTIONALLYCOUPLEDFORCE_H
