/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LAPLACIANSPLIT_H
#define LAPLACIANSPLIT_H

#include "KernelGrad.h"

// Forward Declarations
class LaplacianSplit;

template <>
InputParameters validParams<LaplacianSplit>();

/**
 * Split with a variable that holds the Laplacian of the phase field.
 */
class LaplacianSplit : public KernelGrad
{
public:
  LaplacianSplit(const InputParameters & parameters);

protected:
  virtual RealGradient precomputeQpResidual();
  virtual RealGradient precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const unsigned int _var_c;
  const VariableGradient & _grad_c;
};

#endif // LAPLACIANSPLIT_H
