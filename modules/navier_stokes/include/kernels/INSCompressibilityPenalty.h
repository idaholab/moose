/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef INSCOMPRESSIBILITYPENALTY_H
#define INSCOMPRESSIBILITYPENALTY_H

#include "Kernel.h"

// Forward Declarations
class INSCompressibilityPenalty;

template <>
InputParameters validParams<INSCompressibilityPenalty>();

/**
 * The penalty term may be used when Dirichlet boundary condition is applied to the entire boundary.
 */
class INSCompressibilityPenalty : public Kernel
{
public:
  INSCompressibilityPenalty(const InputParameters & parameters);

  virtual ~INSCompressibilityPenalty() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // penalty value.
  // smaller leads to more accurate solution, but the resulting system is also more difficult to
  // solve
  Real _penalty;
};

#endif /* INSMASSARTIFICIALCOMPRESSIBILITY_H */
