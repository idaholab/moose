//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDMATERIAL2_H_
#define COUPLEDMATERIAL2_H_

#include "Material.h"

class CoupledMaterial2;

template <>
InputParameters validParams<CoupledMaterial2>();

/**
 * A material that couples a material property
 */
class CoupledMaterial2 : public Material
{
public:
  CoupledMaterial2(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  MaterialPropertyName _mat_prop_name;
  MaterialProperty<Real> & _mat_prop;

  const MaterialProperty<Real> & _coupled_mat_prop;
  const MaterialProperty<Real> & _coupled_mat_prop2;
};

#endif // COUPLEDMATERIAL2_H
