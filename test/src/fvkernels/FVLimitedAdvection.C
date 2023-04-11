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

registerMooseObject("MooseTestApp", FVLimitedAdvection);

using namespace Moose::FV;

InputParameters
FVLimitedAdvection::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription("Residual contribution from advection operator for the finite volume "
                             "method with a flux limiter.");
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");

  params.addParam<MooseEnum>(
      "limiter", moose_limiter_type, "The limiter to use for the advected quantity.");
  return params;
}

FVLimitedAdvection::FVLimitedAdvection(const InputParameters & params)
  : FVFluxKernel(params),
    _velocity(getParam<RealVectorValue>("velocity")),
    _limiter(Limiter<ADReal>::build(LimiterType(int(getParam<MooseEnum>("limiter"))))),
    _grad_u_elem(_var.adGradSln()),
    _grad_u_neighbor(_var.adGradSlnNeighbor())
{
}

ADReal
FVLimitedAdvection::computeQpResidual()
{
  const bool elem_is_upwind = _velocity * _normal >= 0;
  const auto face =
      makeFace(*_face_info, LimiterType(int(getParam<MooseEnum>("limiter"))), elem_is_upwind);
  ADReal phi_f = _var(face, Moose::currentState());

  return _normal * _velocity * phi_f;
}
