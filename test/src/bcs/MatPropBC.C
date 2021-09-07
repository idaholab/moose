//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatPropBC.h"

registerMooseObject("MooseTestApp", MatPropBC);

InputParameters
MatPropBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "mat_prop", "The material property that will provide this residual.");
  params.addParam<bool>("use_preinitd_data",
                        false,
                        "Whether to do on the fly computation of variable data or assume that "
                        "variable data has already been pre-initialized");
  return params;
}

MatPropBC::MatPropBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _coef(getFunctorMaterialProperty<ADReal>("mat_prop")),
    _use_preinitd_data(getParam<bool>("use_preinitd_data"))
{
}

ADReal
MatPropBC::computeQpResidual()
{
  if (_use_preinitd_data)
    return _test[_i][_qp] *
           _coef(std::make_tuple(Moose::ElementType::Element, _qp, _current_elem->subdomain_id()));
  else
    return _test[_i][_qp] * _coef(std::make_tuple(_current_elem, _current_side, _qp, _qrule));
}
