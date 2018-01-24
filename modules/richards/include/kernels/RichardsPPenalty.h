/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSPPENALTY
#define RICHARDSPPENALTY

#include "Kernel.h"

// Forward Declarations
class RichardsPPenalty;

template <>
InputParameters validParams<RichardsPPenalty>();

/**
 * Kernel = a*(lower - variable) for variable<lower, and zero otherwise
 * This is an attempt to enforce variable>=lower
 */
class RichardsPPenalty : public Kernel
{
public:
  RichardsPPenalty(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  /// Kernel = a*(_lower - variable) for variable<lower and zero otherwise
  Real _a;

  /// Kernel = a*(_lower - variable) for variable<lower and zero otherwise
  const VariableValue & _lower;

  /// moose variable number of the _lower variable (needed for OffDiagJacobian)
  unsigned int _lower_var_num;
};

#endif // RICHARDSPPENALTY
