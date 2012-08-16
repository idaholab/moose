#ifndef SOLIDMATERIAL_H
#define SOLIDMATERIAL_H

#include "Material.h"

// Forward Declarations
class SolidMaterial;

template<>
InputParameters validParams<SolidMaterial>();

/**
 * A material class for the R7 app.
 */
class SolidMaterial : public Material
{
public:
  SolidMaterial(const std::string & name, InputParameters parameters);
  
protected:

  /**
   * The main Material interface to be specialized
   */
  virtual void computeProperties();
  virtual void gapProperties(Real & K, Real & Rho, Real & Cp, Real Temp);
  virtual void fuelProperties(Real & K, Real & Rho, Real & Cp, Real Temp);
  virtual void cladProperties(Real & K, Real & Rho, Real & Cp, Real Temp);

  /**
   * The Material base class gets the residualSetup() interface from
   * the SetupInterface class.  If we do nothing, this class is empty.
   * If we put something here, we should be able to do some setup 
   * before each residual
   */
  virtual void residualSetup();

  // Material properties exposed to kernels using this material object
  
  /// The solid material properties 
  MaterialProperty<Real> & _thermal_conductivity;
  MaterialProperty<Real> & _specific_heat;
  MaterialProperty<Real> & _density;


  /// (Required) Coupled variables
  VariableValue & _tw;
  std::string _name_of_hs;
};

#endif // R7MATERIAL_H
