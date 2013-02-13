#ifndef SOLIDMATERIAL_H
#define SOLIDMATERIAL_H

#include "Material.h"

// Forward Declarations
class SolidMaterial;

template<>
InputParameters validParams<SolidMaterial>();

/**
 * A class to define materials for the solid structures in the RELAP-7 application.
 */
class SolidMaterial : public Material
{
public:
  SolidMaterial(const std::string & name, InputParameters parameters);
  
protected:
  virtual void computeQpProperties();

  /// The solid material properties 
  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _specific_heat;
  MaterialProperty<Real> & _density;

  /// Temperature in the solid structure
  VariableValue & _temp;
  /// Functions
  Function & _k;
  Function & _Cp;
  Function & _rho;
};

#endif // R7MATERIAL_H
