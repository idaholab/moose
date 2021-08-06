//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumFrictionCorrection.h"
#include "INSFVRhieChowInterpolator.h"
#include "NS.h"
#include "SystemBase.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumFrictionCorrection);

InputParameters
PINSFVMomentumFrictionCorrection::validParams()
{
  auto params = INSFVFluxKernel::validParams();
  params.addClassDescription(
      "Computes the diffusion kernel to avoid pressure-driven oscillations.");
  params.addParam<MooseFunctorName>("Darcy_name", "Name of the Darcy coefficients property.");
  params.addParam<MooseFunctorName>("Forchheimer_name",
                                    "Name of the Forchheimer coefficients property.");
  params.addParam<MooseFunctorName>(NS::porosity, NS::porosity, "The porosity");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The density.");
  params.addRangeCheckedParam<Real>("consistent_scaling",
                                    1,
                                    "consistent_scaling >= 0",
                                    "Smoothing scaling parameter to control "
                                    "collocated mesh oscillations");
  return params;
}

PINSFVMomentumFrictionCorrection::PINSFVMomentumFrictionCorrection(const InputParameters & params)
  : INSFVFluxKernel(params),
    _cL(isParamValid("Darcy_name") ? &getFunctor<ADRealVectorValue>("Darcy_name") : nullptr),
    _cQ(isParamValid("Forchheimer_name") ? &getFunctor<ADRealVectorValue>("Forchheimer_name")
                                         : nullptr),
    _use_Darcy_friction_model(isParamValid("Darcy_name")),
    _use_Forchheimer_friction_model(isParamValid("Forchheimer_name")),
    _eps(getFunctor<ADReal>(NS::porosity)),
    _rho(getFunctor<ADReal>(NS::density)),
    _consistent_scaling(getParam<Real>("consistent_scaling"))
{
  if (!_use_Darcy_friction_model && !_use_Forchheimer_friction_model)
    mooseError("At least one friction model needs to be specified.");
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

void
PINSFVMomentumFrictionCorrection::gatherRCData(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  _face_type = fi.faceType(_var.name());

#ifdef MOOSE_GLOBAL_AD_INDEXING
  using namespace Moose::FV;

  const auto elem_face = elemFromFace();
  const auto neighbor_face = neighborFromFace();

  ADReal friction_term_elem = 0;
  ADReal friction_term_neighbor = 0;

  if (_use_Darcy_friction_model)
  {
    friction_term_elem += (*_cL)(elem_face)(_index)*_rho(elem_face) / _eps(elem_face);
    friction_term_neighbor +=
        (*_cL)(neighbor_face)(_index)*_rho(neighbor_face) / _eps(neighbor_face);
  }
  if (_use_Forchheimer_friction_model)
  {
    friction_term_elem += (*_cQ)(elem_face)(_index)*_rho(elem_face) / _eps(elem_face);
    friction_term_neighbor +=
        (*_cQ)(neighbor_face)(_index)*_rho(neighbor_face) / _eps(neighbor_face);
  }

  Point _face_centroid = _face_info->faceCentroid();
  Point _elem_centroid = _face_info->elemCentroid();
  Point _neighbor_centroid = _face_info->neighborCentroid();

  Real geometric_factor = _consistent_scaling * (_neighbor_centroid - _face_centroid).norm() *
                          (_elem_centroid - _face_centroid).norm();

  // Compute the diffusion driven by the velocity gradient
  // Interpolate viscosity divided by porosity on the face
  ADReal diff_face;
  interpolate(Moose::FV::InterpMethod::Average,
              diff_face,
              friction_term_elem * geometric_factor,
              friction_term_neighbor * geometric_factor,
              *_face_info,
              true);

  // Compute face superficial velocity gradient
  auto dudn =
      _var.gradient(Moose::FV::makeCDFace(*_face_info, faceArgSubdomains())) * _face_info->normal();

  if (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    const auto dof_number = _face_info->elem().dof_number(_sys.number(), _var.number(), 0);
    // A gradient is a linear combination of degrees of freedom so it's safe to straight-up index
    // into the derivatives vector at the dof we care about
    ADReal ae = dudn.derivatives()[dof_number];
    ae *= -diff_face;
    _rc_uo.addToA(&fi.elem(), _index, ae * (fi.faceArea() * fi.faceCoord()));
  }
  if (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
      _face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    const auto dof_number = _face_info->neighbor().dof_number(_sys.number(), _var.number(), 0);
    ADReal an = dudn.derivatives()[dof_number];
    an *= diff_face;
    _rc_uo.addToA(fi.neighborPtr(), _index, an * (fi.faceArea() * fi.faceCoord()));
  }

  const auto strong_resid = -diff_face * dudn;

  processResidual(strong_resid * (fi.faceArea() * fi.faceCoord()));
#endif
}
