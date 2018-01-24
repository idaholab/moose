/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef HHPFCRFF_H
#define HHPFCRFF_H

#include "KernelValue.h"

// Forward Declarations
class HHPFCRFF;

template <>
InputParameters validParams<HHPFCRFF>();

/**
 * TODO: This Kernel needs Documentation!!!
 */
class HHPFCRFF : public KernelValue
{
public:
  HHPFCRFF(const InputParameters & parameters);

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const Real _kernel_sign;

  const MaterialProperty<Real> & _prop;

  const bool _has_coupled_var;
  const VariableValue * _coupled_var;
  const unsigned int _coupled_var_var;
};

#endif // HHFPCRFF_H
