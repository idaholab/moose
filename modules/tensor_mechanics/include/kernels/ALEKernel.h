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

template<>
InputParameters validParams<ALEKernel>();

class ALEKernel : public Kernel
{
public:
  ALEKernel(const InputParameters & parameters);

protected:
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);

  Assembly & _assembly_undisplaced;
  const VariablePhiGradient & _grad_phi_undisplaced;
};

#endif //ALEKERNEL
