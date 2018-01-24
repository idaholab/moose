//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDFORCELAGGEDLAGGED_H
#define COUPLEDFORCELAGGEDLAGGED_H

#include "Kernel.h"

// Forward Declaration
class CoupledForceLagged;

template <>
InputParameters validParams<CoupledForceLagged>();

/**
 * CoupledForce using values from previous Newton iterate
 */
class CoupledForceLagged : public Kernel
{
public:
  CoupledForceLagged(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _v_var;
  const VariableValue & _v;
};

#endif // COUPLEDFORCELAGGEDLAGGED_H
