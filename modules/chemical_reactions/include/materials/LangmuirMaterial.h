#ifndef LANGMUIRMATERIAL_H
#define LANGMUIRMATERIAL_H

#include "Material.h"

//Forward Declarations
class LangmuirMaterial;

template<>
InputParameters validParams<LangmuirMaterial>();

/**
 * Holds Langmuir parameters associated with desorption
 * Calculates equilibrium concentrations for use by kernels
 */
class LangmuirMaterial : public Material
{
public:
  LangmuirMaterial(const std::string & name,
                  InputParameters parameters);

protected:

  /// computes time constants and equilibrium concentrations
  virtual void computeQpProperties();


private:

  /// base desorption time constant
  Real _mat_desorption_time_const;

  /// base adsorption time constant
  Real _mat_adsorption_time_const;

  /// overall desorption time constant = _mat_desorption_time_const/_desorption_time_const_change
  VariableValue * _desorption_time_const_change;

  /// overall adsorption time constant = _mat_adsorption_time_const/_adsorption_time_const_change
  VariableValue * _adsorption_time_const_change;

  /// langmuir density
  Real _mat_langmuir_density;

  /// langmuir pressure
  Real _mat_langmuir_pressure;

  /// porespace pressure (or partial pressure if multiphase flow scenario)
  VariableValue * _pressure;


  /// reciprocal of the overal desorption time constant (kernels use this)
  MaterialProperty<Real> & _one_over_desorption_time_const;

  /// reciprocal of the overal adsorption time constant (kernels use this)
  MaterialProperty<Real> & _one_over_adsorption_time_const;

  /// equilibrium concentration according to Langmuir theory
  MaterialProperty<Real> & _equilib_conc;

  /// derivative of equilibrium concentration wrt _pressure
  MaterialProperty<Real> & _equilib_conc_prime;

};

#endif //LANGMUIRMATERIAL_H
