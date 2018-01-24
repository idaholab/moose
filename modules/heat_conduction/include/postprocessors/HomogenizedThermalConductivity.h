//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
