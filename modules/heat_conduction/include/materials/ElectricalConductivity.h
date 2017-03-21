#ifndef ELECTRICALCONDUCTIVITY_H
#define ELECTRICALCONDUCTIVITY_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

// Forward Declarations
class ElectricalConductivity;

template <>
InputParameters validParams<ElectricalConductivity>();

/**
 * Calculates resistivity and electrical conductivity as a function of temperature.
 * It is assumed that resistivity varies linearly with temperature.
 */
class ElectricalConductivity : public DerivativeMaterialInterface<Material>
{
public:
  ElectricalConductivity(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

private:
  const Real _length_scale;
  const Real _ref_resis;
  const Real _temp_coeff;
  const Real _ref_temp;
  const VariableValue & _T;

  std::string _base_name;
  MaterialProperty<Real> & _electric_conductivity;
  MaterialProperty<Real> & _delectric_conductivity_dT;
};

#endif // ELECTRICALCONDUCTIVITY_H
