//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMaterialPropertyAdvection.h"

registerADMooseObject("MooseApp", FVMaterialPropertyAdvection);

InputParameters
FVMaterialPropertyAdvection::validParams()
{
  InputParameters params = FVMatAdvection::validParams();
  params.addRequiredParam<MaterialPropertyName>("advected_quantity", "The quantity being advected");
  return params;
}

FVMaterialPropertyAdvection::FVMaterialPropertyAdvection(const InputParameters & params)
  : FVMatAdvection(params),
    _adv_quant_elem(getADMaterialProperty<Real>("advected_quantity")),
    _adv_quant_neighbor(getNeighborADMaterialProperty<Real>("advected_quantity"))
{
}

const ADReal &
FVMaterialPropertyAdvection::advQuantityElem()
{
  return _adv_quant_elem[_qp];
}

const ADReal &
FVMaterialPropertyAdvection::advQuantityNeighbor()
{
  return _adv_quant_neighbor[_qp];
}
