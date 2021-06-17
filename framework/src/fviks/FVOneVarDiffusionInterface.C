//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVOneVarDiffusionInterface.h"
#include "FVUtils.h"

registerMooseObject("MooseApp", FVOneVarDiffusionInterface);

InputParameters
FVOneVarDiffusionInterface::validParams()
{
  InputParameters params = FVInterfaceKernel::validParams();
  params.addClassDescription(
      "Computes residual for diffusion operator across an interface for finite volume method.");
  params.addRequiredParam<MaterialPropertyName>("coeff1",
                                                "The diffusion coefficient on the 1st subdomains");
  params.addRequiredParam<MaterialPropertyName>("coeff2",
                                                "The diffusion coefficient on the 2nd subdomains");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

FVOneVarDiffusionInterface::FVOneVarDiffusionInterface(const InputParameters & params)
  : FVInterfaceKernel(params),
    _coeff1_elem(getADMaterialProperty<Real>("coeff1")),
    _coeff2_elem(getADMaterialProperty<Real>("coeff2")),
    _coeff1_neighbor(getNeighborADMaterialProperty<Real>("coeff1")),
    _coeff2_neighbor(getNeighborADMaterialProperty<Real>("coeff2"))
{
  if (&var1() != &var2())
    paramError("variable2",
               name(),
               " is only designed to work with the same variable on both sides of an interface.");
}

ADReal
FVOneVarDiffusionInterface::computeQpResidual()
{
  const auto & coef_elem = elemIsOne() ? _coeff1_elem : _coeff2_elem;
  const auto & coef_neighbor = elemIsOne() ? _coeff2_neighbor : _coeff1_neighbor;

  const auto & grad = var1().adGradSln(*_face_info);

  ADReal coef;
  interpolate(Moose::FV::InterpMethod::Average,
              coef,
              coef_elem[_qp],
              coef_neighbor[_qp],
              *_face_info,
              true);

  return _normal * -coef * grad;
}
