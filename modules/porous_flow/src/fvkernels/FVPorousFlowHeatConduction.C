//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPorousFlowHeatConduction.h"
#include "PorousFlowDictator.h"

registerADMooseObject("PorousFlowApp", FVPorousFlowHeatConduction);

InputParameters
FVPorousFlowHeatConduction::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addRequiredParam<UserObjectName>("PorousFlowDictator",
                                          "The PorousFlowDictator UserObject");
  params.set<unsigned short>("ghost_layers") = 2;
  params.addClassDescription("Conductive heat flux");
  return params;
}

FVPorousFlowHeatConduction::FVPorousFlowHeatConduction(const InputParameters & params)
  : FVFluxKernel(params),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _lambda_element(getADMaterialProperty<RealTensorValue>("PorousFlow_thermal_conductivity_qp")),
    _lambda_neighbor(
        getNeighborADMaterialProperty<RealTensorValue>("PorousFlow_thermal_conductivity_qp")),
    _temperature_element(getADMaterialProperty<Real>("PorousFlow_temperature_qp")),
    _temperature_neighbor(getNeighborADMaterialProperty<Real>("PorousFlow_temperature_qp")),
    _grad_T(getADMaterialProperty<RealGradient>("PorousFlow_grad_temperature_qp"))
{
}

ADReal
FVPorousFlowHeatConduction::computeQpResidual()
{
  ADRealGradient gradT;
  ADRealTensorValue coeff;

  // If we are on a boundary face, use the gradient computed in _grad_T
  if (onBoundary(*_face_info))
  {
    gradT = -_grad_T[_qp];

    coeff = _lambda_element[_qp];
  }
  else
  {
    // If we are on an internal face, calculate the temperature gradient explicitly
    const auto & T_elem = _temperature_element[_qp];
    const auto & T_neighbor = _temperature_neighbor[_qp];

    gradT = (T_elem - T_neighbor) * _face_info->eCN() / _face_info->dCNMag();

    interpolate(Moose::FV::InterpMethod::Average,
                coeff,
                _lambda_element[_qp],
                _lambda_neighbor[_qp],
                gradT,
                *_face_info,
                true);
  }

  return coeff * gradT * _normal;
}
