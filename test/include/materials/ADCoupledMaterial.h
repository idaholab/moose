//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef ADCOUPLEDMATERIAL_H_
#define ADCOUPLEDMATERIAL_H_

#include "Material.h"

class ADCoupledMaterial;

template <>
InputParameters validParams<ADCoupledMaterial>();

/**
 * A material that couples a material property
 */
class ADCoupledMaterial : public Material
{
public:
  ADCoupledMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  std::string _mat_prop_name;
  MaterialProperty<ADReal> & _mat_prop;

  const ADVariableValue & _coupled_var;

  // const MaterialProperty<ADReal> & _coupled_mat_prop;
};

#endif // ADCOUPLEDMATERIAL_H
