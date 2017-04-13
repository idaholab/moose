/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HOMOGENIZEDTHERMALCONDUCTIVITY_H
#define HOMOGENIZEDTHERMALCONDUCTIVITY_H

#include "ElementAverageValue.h"

/**
 * Homogenization of Temperature-Dependent Thermal Conductivity in Composite
 * Materials, Journal of Thermophysics and Heat Transfer, Vol. 15, No. 1,
 * January-March 2001.
 */
class HomogenizedThermalConductivity : public ElementAverageValue
{
public:
  HomogenizedThermalConductivity(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void threadJoin(const UserObject & y);

protected:
  virtual Real computeQpIntegral();

private:
  const VariableGradient & _grad_temp_x;
  const VariableGradient & _grad_temp_y;
  const VariableGradient & _grad_temp_z;
  const unsigned int _component;
  const MaterialProperty<Real> & _diffusion_coefficient;
  Real _volume;
  Real _integral_value;
  const Real _scale;
};

template <>
InputParameters validParams<HomogenizedThermalConductivity>();

#endif // HOMOGENIZEDTHERMALCONDUCTIVITY_H
