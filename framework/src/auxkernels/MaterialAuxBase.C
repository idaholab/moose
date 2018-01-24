//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MaterialAuxBase.h"

template <>
InputParameters
validParams<MaterialAuxBase<>>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<MaterialPropertyName>("property", "The scalar material property name");
  params.addParam<Real>(
      "factor", 1, "The factor by which to multiply your material property for visualization");
  params.addParam<Real>(
      "offset", 0, "The offset to add to your material property for visualization");
  return params;
}
