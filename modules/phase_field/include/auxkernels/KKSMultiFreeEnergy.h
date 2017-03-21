/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef KKSMULTIFREEENERGY_H
#define KKSMULTIFREEENERGY_H

#include "TotalFreeEnergyBase.h"
#include "Material.h"

// Forward Declarations
class KKSMultiFreeEnergy;

template <>
InputParameters validParams<KKSMultiFreeEnergy>();

/**
 * Compute the free energy in the multi-phase KKS Model
 * \f$ F = \sum_i h_i F_i +  + wg_i + \frac{\kappa}{2}|\eta_i|^2 \f$
 */
class KKSMultiFreeEnergy : public TotalFreeEnergyBase
{
public:
  KKSMultiFreeEnergy(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Names of free energy functions for each phase \f$ F_j \f$
  std::vector<MaterialPropertyName> _Fj_names;
  const unsigned int _num_j;

  /// Values of the free energy functions for each phase \f$ F_j \f$
  std::vector<const MaterialProperty<Real> *> _prop_Fj;

  /// Switching function names
  std::vector<MaterialPropertyName> _hj_names;

  /// Values of the switching functions for each phase \f$ h_j \f$
  std::vector<const MaterialProperty<Real> *> _prop_hj;

  /// Barrier function names
  std::vector<MaterialPropertyName> _gj_names;

  /// Values of the barrier functions for each phase \f$ g_j \f$
  std::vector<const MaterialProperty<Real> *> _prop_gj;

  /// Barrier term height
  const Real _w;

  /// Gradient interface free energy coefficients
  std::vector<const MaterialProperty<Real> *> _kappas;
};

#endif // KKSMULTIFREEENERGY_H
