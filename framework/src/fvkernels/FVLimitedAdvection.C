//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVLimitedAdvection.h"
#include "Limiter.h"

registerMooseObject("MooseApp", FVLimitedAdvection);

using namespace Moose::FV;

InputParameters
FVLimitedAdvection::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription(
      "Residual contribution from advection operator for finite volume method.");
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");

  params.addParam<MooseEnum>(
      "limiter", moose_limiter_type, "The limiter to use for the advected quantity.");
  return params;
}

FVLimitedAdvection::FVLimitedAdvection(const InputParameters & params)
  : FVFluxKernel(params),
    _velocity(getParam<RealVectorValue>("velocity")),
    _limiter(Limiter::build(LimiterType(int(getParam<MooseEnum>("limiter"))))),
    _grad_u_elem(_var.adGradSln()),
    _grad_u_neighbor(_var.adGradSlnNeighbor())
{
}

ADReal
FVLimitedAdvection::computeQpResidual()
{
  const bool elem_is_up = _velocity * _normal >= 0;
  const auto & phi_C = elem_is_up ? _u_elem[_qp] : _u_neighbor[_qp];
  const auto & phi_D = elem_is_up ? _u_neighbor[_qp] : _u_elem[_qp];
  const auto & grad_C = elem_is_up ? _grad_u_elem[_qp] : _grad_u_neighbor[_qp];

  const auto phi_f = interpolate(*_limiter, phi_C, phi_D, grad_C, *_face_info, elem_is_up);

  return _normal * _velocity * phi_f;
}
