/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef KKSGLOBALFREEENERGY_H
#define KKSGLOBALFREEENERGY_H

#include "TotalFreeEnergyBase.h"
#include "Material.h"

// Forward Declarations
class KKSGlobalFreeEnergy;

template <>
InputParameters validParams<KKSGlobalFreeEnergy>();

/**
 * Compute the global free energy in the KKS Model
 * \f$ F = hF_a + (1-h)F_b + wg + \frac{\kappa}{2}|\eta|^2 \f$
 */
class KKSGlobalFreeEnergy : public TotalFreeEnergyBase
{
public:
  KKSGlobalFreeEnergy(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const MaterialProperty<Real> & _prop_fa;
  const MaterialProperty<Real> & _prop_fb;
  const MaterialProperty<Real> & _prop_h;
  const MaterialProperty<Real> & _prop_g;

  /// Barrier term height
  const Real _w;

  /// Gradient interface free energy coefficients
  std::vector<const MaterialProperty<Real> *> _kappas;
};

#endif // KKSGLOBALFREEENERGY_H
