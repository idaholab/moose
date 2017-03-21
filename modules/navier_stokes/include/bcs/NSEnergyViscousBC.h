/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSENERGYVISCOUSBC_H
#define NSENERGYVISCOUSBC_H

#include "NSIntegratedBC.h"
#include "NSViscStressTensorDerivs.h"
#include "NSTemperatureDerivs.h"

// Forward Declarations
class NSEnergyViscousBC;

template <>
InputParameters validParams<NSEnergyViscousBC>();

/**
 * This class corresponds to the viscous part of the "natural"
 * boundary condition for the energy equation, i.e.
 *
 * int_{Gamma} n . (- k*grad(T) - tau*u) v
 *
 * A typical use for this kernel would be a subsonic outflow BC in
 * which the pressure is specified.  In this case, the
 * residual and Jacobian contrbutions of the k*grad(T) and
 * n.tau*u terms are computed and added to the matrix/rhs.
 */
class NSEnergyViscousBC : public NSIntegratedBC
{
public:
  NSEnergyViscousBC(const InputParameters & parameters);

protected:
  /**
   * Just like other kernels, we must overload the Residual and Jacobian contributions...
   */
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled gradients
  const VariableGradient & _grad_temperature;

  // Material properties
  const MaterialProperty<Real> & _thermal_conductivity;

  // An object for computing viscous stress tensor derivatives.
  // Constructed via a reference to ourself so we can access all of our data.
  NSViscStressTensorDerivs<NSEnergyViscousBC> _vst_derivs;

  // Declare ourselves friend to the helper class.
  template <class U>
  friend class NSViscStressTensorDerivs;

  // A helper object for computing temperature gradient and Hessians.
  // Constructed via a reference to ourself so we can access all of our data.
  NSTemperatureDerivs<NSEnergyViscousBC> _temp_derivs;

  // Declare ourselves a friend to the helper class
  template <class U>
  friend class NSTemperatureDerivs;

  // Single vector to refer to all gradients.  Initialized in
  // the ctor.
  std::vector<const VariableGradient *> _gradU;
};

#endif // NSENERGYVISCOUSBC_H
