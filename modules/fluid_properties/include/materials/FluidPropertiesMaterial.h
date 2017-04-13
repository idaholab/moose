/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef FLUIDPROPERTIESMATERIAL_H
#define FLUIDPROPERTIESMATERIAL_H

#include "Material.h"

class FluidPropertiesMaterial;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<FluidPropertiesMaterial>();

/**
 * Computes values of pressure and its derivatives using (u, v) formulation
 */
class FluidPropertiesMaterial : public Material
{
public:
  FluidPropertiesMaterial(const InputParameters & parameters);
  virtual ~FluidPropertiesMaterial();

protected:
  virtual void computeQpProperties();

  /// Specific internal energy
  const VariableValue & _e;
  /// Specific volume
  const VariableValue & _v;
  /// Pressure
  MaterialProperty<Real> & _p;
  /// Temperature
  MaterialProperty<Real> & _T;
  /// Sound speed
  MaterialProperty<Real> & _c;
  MaterialProperty<Real> & _cp;
  MaterialProperty<Real> & _cv;
  MaterialProperty<Real> & _mu;
  MaterialProperty<Real> & _k;

  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;
};

#endif /* FLUIDPROPERTIESMATERIAL_H */
