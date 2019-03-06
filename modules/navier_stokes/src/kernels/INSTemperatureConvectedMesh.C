//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSTemperatureConvectedMesh.h"

registerADMooseObject("MooseApp", INSTemperatureConvectedMesh);

defineADValidParams(
    INSTemperatureConvectedMesh,
    ADTimeKernel,
    params.addClassDescription(
        "Corrects the convective derivative for situations in which the fluid mesh is dynamic.");
    params.addRequiredCoupledVar("disp_x", "The x displacement");
    params.addCoupledVar("disp_y", "The y displacement");
    params.addCoupledVar("disp_z", "The z displacement");
    params.addParam<MaterialPropertyName>("rho_name", "rho", "The name of the density");
    params.addParam<MaterialPropertyName>("cp_name", "cp", "specific heat name"););

template <ComputeStage compute_stage>
INSTemperatureConvectedMesh<compute_stage>::INSTemperatureConvectedMesh(
    const InputParameters & parameters)
  : ADTimeKernel<compute_stage>(parameters),
    _disp_x_dot(adCoupledDot("disp_x")),
    _disp_y_dot(isCoupled("disp_y") ? adCoupledDot("disp_y") : adZeroValue()),
    _disp_z_dot(isCoupled("disp_z") ? adCoupledDot("disp_z") : adZeroValue()),
    _rho(adGetADMaterialProperty<Real>("rho_name")),
    _cp(adGetADMaterialProperty<Real>("cp_name"))
{
}

template <ComputeStage compute_stage>
ADResidual
INSTemperatureConvectedMesh<compute_stage>::precomputeQpResidual()
{
  return -_rho[_qp] * _cp[_qp] *
         VectorValue<typename Moose::RealType<compute_stage>::type>(
             _disp_x_dot[_qp], _disp_y_dot[_qp], _disp_z_dot[_qp]) *
         _grad_u[_qp];
}
