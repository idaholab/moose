/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMRZ_H
#define INSMOMENTUMRZ_H

#include "INSMomentumTractionForm.h"

// Forward Declarations
class INSMomentumRZ;

template<>
InputParameters validParams<INSMomentumRZ>();

/**
 * This class computes momentum equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation.
 */
class INSMomentumRZ : public INSMomentumTractionForm
{
public:
  INSMomentumRZ(const InputParameters & parameters);

  virtual ~INSMomentumRZ(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
};


#endif // INSMOMENTUMRZ_H
