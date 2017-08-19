/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMCHILD_H
#define INSMOMENTUMCHILD_H

#include "INSBase.h"

// Forward Declarations
class INSMomentumChild;

template <>
InputParameters validParams<INSMomentumChild>();

/**
 * This class computes the mass equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation.
 */
class INSMomentumChild : public INSBase
{
public:
  INSMomentumChild(const InputParameters & parameters);

  virtual ~INSMomentumChild() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  unsigned _component;
  bool _stokes_only;
};

#endif // INSMOMENTUMCHILD_H
