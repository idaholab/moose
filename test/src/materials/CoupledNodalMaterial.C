//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledNodalMaterial.h"

registerMooseObject("MooseTestApp", CoupledNodalMaterial);

template <>
InputParameters
validParams<CoupledNodalMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("coupled", "Coupled value");
  MooseEnum lag("CURRENT OLD OLDER", "CURRENT", false);
  params.addParam<MooseEnum>("lag", lag, "Determine the time level of the coupled value");
  return params;
}

CoupledNodalMaterial::CoupledNodalMaterial(const InputParameters & parameters)
  : Material(parameters),
    _lag(getParam<MooseEnum>("lag")),
    _coupled_val(_lag == 0 ? coupledNodalValue("coupled")
                           : (_lag == 1 ? coupledNodalValueOld("coupled")
                                        : coupledNodalValueOlder("coupled")))
{
}

void
CoupledNodalMaterial::computeQpProperties()
{
}
