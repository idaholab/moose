//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConvectiveHeatFluxBC.h"

registerMooseObject("HeatTransferApp", ADConvectiveHeatFluxBC);

InputParameters
ADConvectiveHeatFluxBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addClassDescription(
      "Convective heat transfer boundary condition with temperature and heat "
      "transfer coefficient given by material properties.");
  // Using material properties
  params.addParam<MaterialPropertyName>("T_infinity",
                                        "Material property for far-field temperature");
  params.addParam<MaterialPropertyName>("heat_transfer_coefficient",
                                        "Material property for heat transfer coefficient");
  // Using functors
  params.addParam<MooseFunctorName>("T_infinity_functor", "Functor for far-field temperature");
  params.addParam<MooseFunctorName>("heat_transfer_coefficient_functor",
                                    "Functor for heat transfer coefficient");
  return params;
}

ADConvectiveHeatFluxBC::ADConvectiveHeatFluxBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _T_infinity(isParamValid("T_infinity") ? &getADMaterialProperty<Real>("T_infinity") : nullptr),
    _htc(isParamValid("heat_transfer_coefficient")
             ? &getADMaterialProperty<Real>("heat_transfer_coefficient")
             : nullptr),
    _T_infinity_functor(
        isParamValid("T_infinity_functor") ? &getFunctor<ADReal>("T_infinity_functor") : nullptr),
    _htc_functor(isParamValid("heat_transfer_coefficient_functor")
                     ? &getFunctor<ADReal>("heat_transfer_coefficient_functor")
                     : nullptr)
{
  if (_T_infinity || _htc)
  {
    if (_T_infinity_functor || _htc_functor)
      paramError("T_infinity_functor",
                 "Either material properties or functors should be specified");
    if (!_htc)
      paramError("heat_transfer_coefficient",
                 "Heat transfer coefficient material property must be specified");
    if (!_T_infinity)
      paramError("T_infinity", "Far field temperature material property must be specified");
  }
  else if (_T_infinity_functor || _htc_functor)
  {
    if (!_htc_functor)
      paramError("heat_transfer_coefficient_functor",
                 "Heat transfer coefficient functor must be specified");
    if (!_T_infinity_functor)
      paramError("T_infinity_functor", "Far field temperature functor must be specified");
  }
  else
    paramError("T_infinity",
               "Far field temperature and heat transfer coefficients must be specified");
}

ADReal
ADConvectiveHeatFluxBC::computeQpResidual()
{
  if (_T_infinity)
    return -_test[_i][_qp] * (*_htc)[_qp] * ((*_T_infinity)[_qp] - _u[_qp]);
  else
  {
    const auto time_arg = Moose::currentState();
    const Moose::ElemSideQpArg space_arg = {
        _current_elem, _current_side, _qp, _qrule, _q_point[_qp]};
    return -_test[_i][_qp] * (*_htc_functor)(space_arg, time_arg) *
           ((*_T_infinity_functor)(space_arg, time_arg) - _u[_qp]);
  }
}
