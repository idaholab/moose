//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUCoupledVarNeumannBC.h"

registerMooseObject("MooseApp", GPUCoupledVarNeumannBC);

InputParameters
GPUCoupledVarNeumannBC::validParams()
{
  InputParameters params = GPUIntegratedBC::validParams();
  params.addRequiredCoupledVar("v", "Coupled variable setting the gradient on the boundary.");
  params.addCoupledVar("scale_factor", 1., "Scale factor to multiply the heat flux with");
  params.addParam<Real>(
      "coef", 1.0, "Coefficent ($\\sigma$) multiplier for the coupled force term.");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "$\\frac{\\partial u}{\\partial n}=v$, "
                             "where $v$ is a variable.");
  return params;
}
GPUCoupledVarNeumannBC::GPUCoupledVarNeumannBC(const InputParameters & parameters)
  : GPUIntegratedBC(parameters),
    _coupled_var(GPUCoupledValue("v")),
    _coupled_num(coupled("v")),
    _coef(getParam<Real>("coef")),
    _scale_factor(GPUCoupledValue("scale_factor"))
{
}
