//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVFunctorHeatConductionTimeDerivative.h"

registerMooseObject("HeatTransferApp", FVFunctorHeatConductionTimeDerivative);

InputParameters
FVFunctorHeatConductionTimeDerivative::validParams()
{
  InputParameters params = FVFunctorTimeKernel::validParams();
  params.addClassDescription(
      "AD Time derivative term $\\rho c_p \\frac{\\partial T}{\\partial t}$ of "
      "the heat equation for quasi-constant specific heat $c_p$ and the density $\\rho$.");
  params.addParam<MooseFunctorName>(
      "specific_heat", "specific_heat", "Property name of the specific heat at constant pressure");
  params.addParam<MooseFunctorName>("density", "density", "Property name of the density");
  return params;
}

FVFunctorHeatConductionTimeDerivative::FVFunctorHeatConductionTimeDerivative(
    const InputParameters & parameters)
  : FVFunctorTimeKernel(parameters),
    _specific_heat(getFunctor<ADReal>("specific_heat")),
    _density(getFunctor<ADReal>("density"))
{
}

ADReal
FVFunctorHeatConductionTimeDerivative::computeQpResidual()
{
  // FV kernels dont use the quadrature
  mooseAssert(_q_point.size() == 1, "Only 1 QP per element in FV");
  const Moose::ElemArg elem_arg = {_current_elem, /*correct_skewness=*/true};
  const auto & time_arg = determineState();
  return _specific_heat(elem_arg, time_arg) * _density(elem_arg, time_arg) *
         FVFunctorTimeKernel::computeQpResidual();
}
