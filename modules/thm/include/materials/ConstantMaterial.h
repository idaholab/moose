#ifndef CONSTANTMATERIAL_H
#define CONSTANTMATERIAL_H

#include "DerivativeMaterialInterface.h"

class ConstantMaterial;

template <>
InputParameters validParams<ConstantMaterial>();

/**
 * Constant material with zero-valued derivatives
 */
class ConstantMaterial : public DerivativeMaterialInterface<Material>
{
public:
  ConstantMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const Real & _value;

  const MaterialPropertyName _property_name;

  MaterialProperty<Real> & _property;

  /// Number of variables for which to create zero-valued property derivatives
  const unsigned int _n_derivative_vars;

  /// List of variables for which to create zero-valued property derivatives
  std::vector<const VariableValue *> _derivative_vars;

  /// Derivatives of material property with respect to each variable
  std::vector<MaterialProperty<Real> *> _derivative_properties;
};

#endif /* CONSTANTMATERIAL_H */
