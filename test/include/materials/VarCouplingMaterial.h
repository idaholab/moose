//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VARCOUPLINGMATERIAL_H_
#define VARCOUPLINGMATERIAL_H_

#include "Material.h"

class VarCouplingMaterial;

template <>
InputParameters validParams<VarCouplingMaterial>();

/**
 * A material that couples a variable
 */
class VarCouplingMaterial : public Material
{
public:
  VarCouplingMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void initQpStatefulProperties();

  const VariableValue & _var;
  Real _base;
  Real _coef;
  MaterialProperty<Real> & _diffusion;
  const MaterialProperty<Real> * const _diffusion_old;
};

#endif // VARCOUPLINGMATERIAL_H
