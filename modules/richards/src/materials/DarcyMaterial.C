/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "DarcyMaterial.h"

template <>
InputParameters
validParams<DarcyMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<RealTensorValue>("mat_permeability",
                                           "The permeability tensor (usually in m^2).");
  params.addClassDescription("Material that holds the permeability tensor used in Darcy flow");
  return params;
}

DarcyMaterial::DarcyMaterial(const InputParameters & parameters)
  : Material(parameters),
    _material_perm(getParam<RealTensorValue>("mat_permeability")),
    _permeability(declareProperty<RealTensorValue>("permeability"))
{
}

void
DarcyMaterial::computeQpProperties()
{
  _permeability[_qp] = _material_perm;
}
