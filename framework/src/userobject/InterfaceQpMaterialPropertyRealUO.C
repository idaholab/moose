//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceQpMaterialPropertyRealUO.h"

registerMooseObject("MooseApp", InterfaceQpMaterialPropertyRealUO);

InputParameters
InterfaceQpMaterialPropertyRealUO::validParams()
{
  InputParameters params = InterfaceQpMaterialPropertyBaseUserObject<Real>::validParams();
  params.addClassDescription(
      "Computes the value, rate or increment of a Real Material property across an "
      "interface. The value, rate or increment is computed according to the provided "
      "interface_value_type parameter");
  return params;
}

InterfaceQpMaterialPropertyRealUO::InterfaceQpMaterialPropertyRealUO(
    const InputParameters & parameters)
  : InterfaceQpMaterialPropertyBaseUserObject<Real>(parameters)
{
}
