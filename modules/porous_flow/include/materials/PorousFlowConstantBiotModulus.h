/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWCONSTANTBIOTMODULUS_H
#define POROUSFLOWCONSTANTBIOTMODULUS_H

#include "PorousFlowMaterialVectorBase.h"

// Forward Declarations
class PorousFlowConstantBiotModulus;

template <>
InputParameters validParams<PorousFlowConstantBiotModulus>();

/**
 * Material designed to provide a time-invariant
 * Biot Modulus, M, where
 * 1 / M = (1 - alpha) * (alpha - phi) * C + phi / Kf .
 * Here
 * alpha = Biot coefficient (assumed constant)
 * phi = initial value of porosity
 * C = drained porous-solid bulk compliance (1 / bulk modulus)
 * Kf = fluid bulk modulus (assumed constant)
 */
class PorousFlowConstantBiotModulus : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowConstantBiotModulus(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Biot coefficient
  const Real _biot_coefficient;

  /// Fluid bulk modulus
  const Real _fluid_bulk_modulus;

  /// Solid bulk compliance
  const Real _solid_bulk_compliance;

  /// porosity at the nodes or quadpoints.  Only the initial value is ever used
  const MaterialProperty<Real> & _porosity;

  /// Computed Biot modulus
  MaterialProperty<Real> & _biot_modulus;

  /// Old value of Biot modulus.  This variable is necessary in order to keep Biot modulus constant even if porosity is changing.
  const MaterialProperty<Real> & _biot_modulus_old;
};

#endif // POROUSFLOWCONSTANTBIOTMODULUS_H
