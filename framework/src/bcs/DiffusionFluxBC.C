//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionFluxBC.h"

registerMooseObject("MooseApp", DiffusionFluxBC);

InputParameters
DiffusionFluxBC::validParams()
{
  InputParameters params = FluxBC::validParams();
  params.addClassDescription(
      "Computes a boundary residual contribution consistent with the Diffusion Kernel. "
      "Does not impose a boundary condition; instead computes the boundary "
      "contribution corresponding to the current value of grad(u) and accumulates "
      "it in the residual vector.");
  return params;
}

DiffusionFluxBC::DiffusionFluxBC(const InputParameters & parameters) : FluxBC(parameters) {}

RealGradient
DiffusionFluxBC::computeQpFluxResidual()
{
  return _grad_u[_qp];
}

RealGradient
DiffusionFluxBC::computeQpFluxJacobian()
{
  return _grad_phi[_j][_qp];
}
