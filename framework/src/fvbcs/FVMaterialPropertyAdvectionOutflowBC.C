//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVMaterialPropertyAdvectionOutflowBC.h"

registerADMooseObject("MooseApp", FVMaterialPropertyAdvectionOutflowBC);

InputParameters
FVMaterialPropertyAdvectionOutflowBC::validParams()
{
  InputParameters params = FVMatAdvectionOutflowBC::validParams();
  params.addRequiredParam<MaterialPropertyName>("advected_quantity",
                                                "The material property being advected");
  return params;
}

FVMaterialPropertyAdvectionOutflowBC::FVMaterialPropertyAdvectionOutflowBC(
    const InputParameters & params)
  : FVMatAdvectionOutflowBC(params), _adv_quant(getADMaterialProperty<Real>("advected_quantity"))
{
}

const ADReal &
FVMaterialPropertyAdvectionOutflowBC::advQuantity()
{
  return _adv_quant[_qp];
}
