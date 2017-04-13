/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef VARIABLEGRADIENTMATERIAL_H
#define VARIABLEGRADIENTMATERIAL_H

#include "Material.h"

class VariableGradientMaterial;

template <>
InputParameters validParams<VariableGradientMaterial>();

/**
 * Set a material property to the norm of the gradient of a non-linear variable
 */
class VariableGradientMaterial : public Material
{
public:
  VariableGradientMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const VariableGradient & _grad;
  MaterialProperty<Real> & _prop;
};

#endif // VARIABLEGRADIENTMATERIAL_H
