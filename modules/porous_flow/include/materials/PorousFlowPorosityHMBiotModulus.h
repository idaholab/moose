/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPOROSITYHMBIOTMODULUS_H
#define POROUSFLOWPOROSITYHMBIOTMODULUS_H

#include "PorousFlowPorosityHM.h"

// Forward Declarations
class PorousFlowPorosityHMBiotModulus;

template <>
InputParameters validParams<PorousFlowPorosityHMBiotModulus>();

/**
 * This Matrial evolves porosity so that the PorousFlow equations match
 * the standard equations of poroelasticity theory with a constant BiotModulus.
 * Even though a constant BiotModulus is not strictly correct, many
 * analytical solutions of poroelasticity theory assume this, so
 * PorousFlowPorosityHMBiotModulus is useful for comparing with those solutions.
 * Otherwise it shouldn't generally be used because PorousFlowPorosityHM
 * is physically more correct.
 */
class PorousFlowPorosityHMBiotModulus : public PorousFlowPorosityHM
{
public:
  PorousFlowPorosityHMBiotModulus(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// old value of porosity
  const MaterialProperty<Real> & _porosity_old;

  /// constant biot modulus
  const Real _biot_modulus;

  /// constant fluid bulk modulus
  const Real _fluid_bulk_modulus;

  /// old value of effective fluid pressure
  const MaterialProperty<Real> & _pf_old;

  /// old value of total volumetric strain
  const MaterialProperty<Real> & _vol_strain_qp_old;

  /// volumetric strain rate
  const MaterialProperty<Real> & _vol_strain_rate_qp;

  /// d(volumetric strain rate)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> & _dvol_strain_rate_qp_dvar;
};

#endif // POROUSFLOWPOROSITYHMBIOTMODULUS_H
