/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSENERGYVISCOUSFLUX_H
#define NSENERGYVISCOUSFLUX_H

#include "NSKernel.h"
#include "NSViscStressTensorDerivs.h"

// Forward Declarations
class NSEnergyViscousFlux;

template <>
InputParameters validParams<NSEnergyViscousFlux>();

/**
 * Viscous flux terms in energy equation.
 */
class NSEnergyViscousFlux : public NSKernel
{
public:
  NSEnergyViscousFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // An object for computing viscous stress tensor derivatives.
  // Constructed via a reference to ourself
  NSViscStressTensorDerivs<NSEnergyViscousFlux> _vst_derivs;

  // Declare ourselves friend to the helper class.
  template <class U>
  friend class NSViscStressTensorDerivs;
};

#endif //  NSENERGYVISCOUSFLUX_H
