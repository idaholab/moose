#ifndef SOLIDMATERIAL_H
#define SOLIDMATERIAL_H

#include "Material.h"
#include "SolidMaterialProperties.h"

// Forward Declarations
class SolidMaterial;

template <>
InputParameters validParams<SolidMaterial>();

/**
 * A class to define materials for the solid structures in the RELAP-7 application.
 */
class SolidMaterial : public Material
{
public:
  SolidMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// The solid material properties
  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _specific_heat;
  MaterialProperty<Real> & _density;

  /// Temperature in the solid structure
  const VariableValue & _temp;
  /// User object with material properties
  const SolidMaterialProperties & _props;
};

#endif // SOLIDMATERIAL_H
