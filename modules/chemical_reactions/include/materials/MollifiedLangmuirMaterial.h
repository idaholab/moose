/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MOLLFIEDLANGMUIRMATERIAL_H
#define MOLLFIEDLANGMUIRMATERIAL_H

#include "Material.h"

// Forward Declarations
class MollifiedLangmuirMaterial;

template <>
InputParameters validParams<MollifiedLangmuirMaterial>();

/**
 * Holds Langmuir parameters associated with desorption
 * Calculates mass-flow rates and derivatives thereof for use by kernels
 */
class MollifiedLangmuirMaterial : public Material
{
public:
  MollifiedLangmuirMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

private:
  /// reciprocal of desorption time constant
  const VariableValue * _one_over_de_time_const;

  /// reciprocal of adsorption time constant
  const VariableValue * _one_over_ad_time_const;

  /// langmuir density
  const Real _langmuir_dens;

  /// langmuir pressure
  const Real _langmuir_p;

  /// concentration of adsorbed fluid in matrix
  const VariableValue * _conc;

  /// porespace pressure (or partial pressure if multiphase flow scenario)
  const VariableValue * _pressure;

  /**
   * mollifying parameter.  the time constants are
   * one_over_time_const * tanh( |_conc - equilib_conc|/(mollifier*_langmuir_dens) )
   * Here equilib_conc = langmuir_dens*_pressure/(_pressure + _langmuir_p)
   * which is the langmuir expression
   */
  const Real _mollifier;

  /// mass flow rate from the matrix = mass flow rate to the porespace
  MaterialProperty<Real> & _mass_rate_from_matrix;

  /// derivative of mass flow rate wrt concentration
  MaterialProperty<Real> & _dmass_rate_from_matrix_dC;

  /// derivative of mass flow rate wrt pressure
  MaterialProperty<Real> & _dmass_rate_from_matrix_dp;
};

#endif // MOLLFIEDLANGMUIRMATERIAL_H
