/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ALEKERNEL_H
#define ALEKERNEL_H

#include "Kernel.h"
#include "Assembly.h"

class ALEKernel;

template <>
InputParameters validParams<ALEKernel>();

class ALEKernel : public Kernel
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
