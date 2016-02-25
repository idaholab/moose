/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LINEARELASTICITYMATERIAL_H
#define LINEARELASTICITYMATERIAL_H

#include "Material.h"


/**
 * Simple material with constant properties.
 */
class LinearElasticityMaterial : public Material
{
public:
  LinearElasticityMaterial(const InputParameters & parameters);

protected:
  virtual void computeProperties();

private:

  bool _has_temp;
  const VariableValue & _temp;

  Real _my_thermal_expansion;
  Real _my_youngs_modulus;
  Real _my_poissons_ratio;
  Real _my_t_ref;

  MaterialProperty<Real> & _thermal_strain;
  MaterialProperty<Real> & _alpha;
  MaterialProperty<Real> & _youngs_modulus;
  MaterialProperty<Real> & _poissons_ratio;
};

template<>
InputParameters validParams<LinearElasticityMaterial>();

#endif
