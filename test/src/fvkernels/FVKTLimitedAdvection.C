//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVKTLimitedAdvection.h"
#include "Limiter.h"

registerMooseObject("MooseTestApp", FVKTLimitedAdvection);

using namespace Moose::FV;

InputParameters
FVKTLimitedAdvection::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription(
      "Residual contribution from advection operator for finite volume method.");
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  params.addRequiredParam<MaterialPropertyName>(
      "max_abs_eig", "The maximum absolute value of the advection Jacobian eigenvalues.");
  params.addParam<MooseEnum>(
      "limiter", moose_limiter_type, "The limiter to use for the advected quantity.");
  params.addParam<bool>("add_artificial_diff", true, "Add artificial diffusion for monotonicity.");
  return params;
}

FVKTLimitedAdvection::FVKTLimitedAdvection(const InputParameters & params)
  : FVFluxKernel(params),
    _velocity(getParam<RealVectorValue>("velocity")),
    _limiter(Limiter<ADReal>::build(LimiterType(int(getParam<MooseEnum>("limiter"))))),
    _cd_limiter(Limiter<ADReal>::build(LimiterType::CentralDifference)),
    _grad_u_elem(_var.adGradSln()),
    _grad_u_neighbor(_var.adGradSlnNeighbor()),
    _max_abs_eig(getADMaterialProperty<Real>("max_abs_eig")),
    _add_artificial_diff(getParam<bool>("add_artificial_diff"))
{
}

ADReal
FVKTLimitedAdvection::computeQpResidual()
{
  const auto phi_avg_f = interpolate(
      *_cd_limiter, _u_elem[_qp], _u_neighbor[_qp], &_grad_u_elem[_qp], *_face_info, true);
  const auto phi_elem =
      interpolate(*_limiter, _u_elem[_qp], _u_neighbor[_qp], &_grad_u_elem[_qp], *_face_info, true);
  const auto phi_neighbor = interpolate(
      *_limiter, _u_neighbor[_qp], _u_elem[_qp], &_grad_u_neighbor[_qp], *_face_info, false);

  auto resid = _normal * _velocity * phi_avg_f;
  if (_add_artificial_diff)
    resid -= 0.5 * _max_abs_eig[_qp] * (phi_neighbor - phi_elem);
  return resid;
}
