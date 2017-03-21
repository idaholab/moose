/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMTIMEDERIVATIVE_H
#define INSMOMENTUMTIMEDERIVATIVE_H

#include "TimeDerivative.h"

// Forward Declarations
class INSMomentumTimeDerivative;

template <>
InputParameters validParams<INSMomentumTimeDerivative>();

/**
 * This class computes the time derivative for the incompressible
 * Navier-Stokes momentum equation.  Could instead use CoefTimeDerivative
 * for this.
 */
class INSMomentumTimeDerivative : public TimeDerivative
{
public:
  INSMomentumTimeDerivative(const InputParameters & parameters);

  virtual ~INSMomentumTimeDerivative() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Parameters
  Real _rho;
};

#endif // INSMOMENTUMTIMEDERIVATIVE_H
