/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef DARCYFLUXPRESSURE_H
#define DARCYFLUXPRESSURE_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

class DarcyFluxPressure;

template <>
InputParameters validParams<DarcyFluxPressure>();

/**
 * Darcy flux: - cond * (Grad P - rho * g)
 * where cond is the hydraulic conductivity, P is fluid pressure,
 * rho is flui density and g is gravity
 */
class DarcyFluxPressure : public DerivativeMaterialInterface<Kernel>
{
public:
  DarcyFluxPressure(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Hydraulic conductivity
  const MaterialProperty<Real> & _cond;

  /// Gravity
  const RealVectorValue _gravity;

  /// Fluid density
  const MaterialProperty<Real> & _density;
};

#endif // DARCYFLUXPRESSURE_H
