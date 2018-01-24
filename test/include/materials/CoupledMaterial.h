//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDMATERIAL_H_
#define COUPLEDMATERIAL_H_

#include "Material.h"

class CoupledMaterial;

template <>
InputParameters validParams<CoupledMaterial>();

/**
 * A material that couples a material property
 */
class CoupledMaterial : public Material
{
public:
  CoupledMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override { _mat_prop[_qp] = 1.0; }
  virtual void computeQpProperties() override;

  std::string _mat_prop_name;
  MaterialProperty<Real> & _mat_prop;

  const MaterialProperty<Real> & _coupled_mat_prop;
};

#endif // COUPLEDMATERIAL_H
