/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef NSPRESSURENEUMANNBC_H
#define NSPRESSURENEUMANNBC_H

#include "NSIntegratedBC.h"
#include "NSPressureDerivs.h"

// Forward Declarations
class NSPressureNeumannBC;

template <>
InputParameters validParams<NSPressureNeumannBC>();

/**
 * This kernel is appropriate for use with a "zero normal flow"
 * boundary condition in the context of the Euler equations.
 * In this situation, the convective term is integrated by parts
 * and the (rho*u)(u.n) term is zero since u.n=0.  Thus all we
 * are left with is the pressure times the normal.
 *
 * For the Navier-Stokes equations, a no-slip boundary condition
 * is probably what you want instead of this... for that use
 * NSImposedVelocityBC instead.
 */
class NSPressureNeumannBC : public NSIntegratedBC
{
public:
  NSPressureNeumannBC(const InputParameters & parameters);

  virtual ~NSPressureNeumannBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled vars
  const VariableValue & _pressure;

  // Required parameters
  unsigned _component;

  // An object for computing pressure derivatives.
  // Constructed via a reference to ourself
  NSPressureDerivs<NSPressureNeumannBC> _pressure_derivs;

  // Declare ourselves friend to the helper class.
  template <class U>
  friend class NSPressureDerivs;

private:
  // Computes the Jacobian value for this term for variable 'm'
  // in the canonical ordering.
  Real computeJacobianHelper(unsigned m);
};

#endif // PRESSURENEUMANNBC_H
