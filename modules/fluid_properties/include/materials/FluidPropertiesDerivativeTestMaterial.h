/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef FLUIDPROPERTIESDERIVATIVETESTMATERIAL_H
#define FLUIDPROPERTIESDERIVATIVETESTMATERIAL_H

#include "Material.h"

class FluidPropertiesDerivativeTestMaterial;
class SinglePhaseFluidPropertiesPT;

template<>
InputParameters validParams<FluidPropertiesDerivativeTestMaterial>();

/**
 * Tests derivatives given in the FluidProperties UserObjects by calculating
 * a finite-difference derivative to compare the provided derivatives to.
 * Note: This material is intended for testing purposes only
 */
class FluidPropertiesDerivativeTestMaterial : public Material
{
public:
  FluidPropertiesDerivativeTestMaterial(const InputParameters & parameters);
  virtual ~FluidPropertiesDerivativeTestMaterial();

protected:
  virtual void computeQpProperties();

  /// Pressure (Pa)
  const VariableValue & _pressure;
  /// Temperature (K)
  const VariableValue & _temperature;

  /// Derivative of density wrt pressure
  MaterialProperty<Real> & _drho_dp;
  /// Derivative of density wrt temperature
  MaterialProperty<Real> & _drho_dT;
  /// Derivative of internal energy wrt pressure
  MaterialProperty<Real> & _de_dp;
  /// Derivative of internal energy wrt temperature
  MaterialProperty<Real> & _de_dT;
  /// Derivative of enthalpy wrt pressure
  MaterialProperty<Real> & _dh_dp;
  /// Derivative of enthalpy wrt temperature
  MaterialProperty<Real> & _dh_dT;
  /// Derivative of viscosity wrt density
  MaterialProperty<Real> & _dmu_drho;
  /// Derivative of viscosity wrt temperature
  MaterialProperty<Real> & _dmu_dT;

  /// Finite difference derivative of density wrt pressure
  MaterialProperty<Real> & _drho_dp_fd;
  /// Finite difference derivative of density wrt temperature
  MaterialProperty<Real> & _drho_dT_fd;
  /// Finite Difference derivative of internal energy wrt pressure
  MaterialProperty<Real> & _de_dp_fd;
  /// Finite difference derivative of internal energy wrt temperature
  MaterialProperty<Real> & _de_dT_fd;
  /// Finite Difference derivative of enthalpy wrt pressure
  MaterialProperty<Real> & _dh_dp_fd;
  /// Finite difference derivative of enthalpy wrt temperature
  MaterialProperty<Real> & _dh_dT_fd;
  /// Finite difference derivative of viscosity wrt density
  MaterialProperty<Real> & _dmu_drho_fd;
  /// Finite difference derivative of viscosity wrt temperature
  MaterialProperty<Real> & _dmu_dT_fd;

  /// Fluid properties UserObject for (p,T) formulation
  const SinglePhaseFluidPropertiesPT & _fp;

  /// Finite difference epsilon
  const Real _eps;
};

#endif /* FLUIDPROPERTIESDERIVATIVETESTMATERIAL_H */
