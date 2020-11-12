#pragma once

#include "Material.h"

/**
 * Stores values of a variable into material properties
 */
template <bool is_ad>
class CoupledVariableValueMaterialTempl : public Material
{
public:
  CoupledVariableValueMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// The name of the material property where the values will be stored
  const MaterialPropertyName & _prop_name;
  /// Storage for the variable values
  GenericMaterialProperty<Real, is_ad> & _prop;
  /// The coupled variable values
  const VariableValue & _value;
  const ADVariableValue & _ad_value;

public:
  static InputParameters validParams();
};

typedef CoupledVariableValueMaterialTempl<false> CoupledVariableValueMaterial;
typedef CoupledVariableValueMaterialTempl<true> ADCoupledVariableValueMaterial;
