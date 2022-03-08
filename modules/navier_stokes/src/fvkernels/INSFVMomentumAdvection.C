//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumAdvection.h"
#include "NS.h"
#include "FVUtils.h"
#include "INSFVRhieChowInterpolator.h"

registerMooseObject("NavierStokesApp", INSFVMomentumAdvection);

InputParameters
INSFVMomentumAdvection::validParams()
{
  InputParameters params = INSFVAdvectionKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density functor");
  params.addClassDescription("Object for advecting momentum, e.g. rho*u");
  return params;
}

INSFVMomentumAdvection::INSFVMomentumAdvection(const InputParameters & params)
  : INSFVAdvectionKernel(params),
    INSFVMomentumResidualObject(*this),
    _rho(getFunctor<ADReal>(NS::density)),
    _rho_u(
        "rho_u",
        [this](const auto & r, const auto & t) -> ADReal { return _rho(r, t) * _var(r, t); },
        std::set<ExecFlagType>({EXEC_ALWAYS}),
        _mesh,
        this->blockIDs())
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
INSFVMomentumAdvection::computeQpResidual()
{
  using namespace Moose::FV;

  const auto v = _rc_vel_provider.getVelocity(_velocity_interp_method, *_face_info, _tid);
  const auto [interp_coeffs, advected] =
      interpCoeffsAndAdvected(_rho_u,
                              makeFace(*_face_info,
                                       limiterType(_advected_interp_method),
                                       MetaPhysicL::raw_value(v) * _normal > 0,
                                       faceArgSubdomains()));

  const auto elem_face = elemFromFace();
  const auto neighbor_face = neighborFromFace();

  const auto rho_elem = _rho(elem_face), rho_neighbor = _rho(neighbor_face);
  const auto var_elem = advected.first / rho_elem, var_neighbor = advected.second / rho_neighbor;

  _ae = _normal * v * rho_elem * interp_coeffs.first;
  // Minus sign because we apply a minus sign to the residual in computeResidual
  _an = -_normal * v * rho_neighbor * interp_coeffs.second;

  return _ae * var_elem - _an * var_neighbor;
}

void
INSFVMomentumAdvection::gatherRCData(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(_var.name());

  const auto saved_velocity_interp_method = _velocity_interp_method;
  _velocity_interp_method = Moose::FV::InterpMethod::Average;
  // Fill-in the coefficients _ae and _an (but without multiplication by A)
  computeQpResidual();
  _velocity_interp_method = saved_velocity_interp_method;

  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    _rc_uo.addToA(&fi.elem(), _index, _ae * (fi.faceArea() * fi.faceCoord()));
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
    _rc_uo.addToA(fi.neighborPtr(), _index, _an * (fi.faceArea() * fi.faceCoord()));
}
