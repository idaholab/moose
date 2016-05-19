/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef INSPENALTYTERM_H_
#define INSPENALTYTERM_H_

#include "Kernel.h"

// Forward Declarations
class INSMassArtificialCompressibility;

template<>
InputParameters validParams<INSMassArtificialCompressibility>();

/**
 * The penalty term may be used when Dirichlet boundary condition is applied to the entire boundary.
 */
class INSMassArtificialCompressibility : public Kernel
{
public:
  INSMassArtificialCompressibility(const InputParameters & parameters);

  virtual ~INSMassArtificialCompressibility(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // penalty value.
  // smaller leads to more accurate solution, but the resulting system is also more difficult to solve
  Real _penalty;
};




#endif /* INSPENALTYTERM_H_ */
