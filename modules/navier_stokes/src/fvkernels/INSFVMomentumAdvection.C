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
    _rho(getFunctor<ADReal>(NS::density))
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
  const auto elem_face = elemFromFace();
  const auto neighbor_face = neighborFromFace();

  const auto v = _rc_vel_provider.getVelocity(_velocity_interp_method, *_face_info, _tid);
  const auto interp_coeffs = Moose::FV::interpCoeffs(_advected_interp_method, *_face_info, true, v);
  _ae = _normal * v * _rho(elem_face) * interp_coeffs.first;
  // Minus sign because we apply a minus sign to the residual in computeResidual
  _an = -_normal * v * _rho(neighbor_face) * interp_coeffs.second;

  return _ae * _var(elem_face) - _an * _var(neighbor_face);
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
