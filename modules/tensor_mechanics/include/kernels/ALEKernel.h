//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ALEKERNEL_H
#define ALEKERNEL_H

#include "Kernel.h"
#include "Assembly.h"
#include "DerivativeMaterialInterface.h"

class ALEKernel;

template <>
InputParameters validParams<ALEKernel>();

class ALEKernel : public DerivativeMaterialInterface<Kernel>
{
public:
  ALEKernel(const InputParameters & parameters);

  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);

protected:
  /// undisplaced problem
  Assembly & _assembly_undisplaced;

  /// Reference to this Kernel's undisplaced MooseVariable object
  MooseVariable & _var_undisplaced;

  ///@{ Shape and test functions on the undisplaced mesh
  const VariablePhiGradient & _grad_phi_undisplaced;
  const VariableTestGradient & _grad_test_undisplaced;
  ///@}
};

#endif // ALEKERNEL_H
