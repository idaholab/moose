//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMassAdvection.h"

registerMooseObject("NavierStokesApp", INSFVMassAdvection);

InputParameters
INSFVMassAdvection::validParams()
{
  auto params = INSFVMomentumAdvection::validParams();
  params.addClassDescription("Object for advecting mass, e.g. rho");
  params.suppressParameter<MaterialPropertyName>("advected_quantity");
  return params;
}

INSFVMassAdvection::INSFVMassAdvection(const InputParameters & params)
  : INSFVMomentumAdvection(params)
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
INSFVMassAdvection::computeQpResidual()
{
  ADRealVectorValue v;
  ADReal face_rho;
  Point normal;

  this->interpolate(_velocity_interp_method, v, _vel_elem[_qp], _vel_neighbor[_qp]);
  if (onBoundary(*_face_info))
  {
    const auto ft = _face_info->faceType(_var.name());
    const bool out_of_elem = (ft == FaceInfo::VarFaceNeighbors::ELEM);
    normal = out_of_elem ? _face_info->normal() : Point(-_face_info->normal());
    face_rho = out_of_elem ? _rho(&_face_info->elem()) : _rho(_face_info->neighborPtr());
  }
  else
  {
    normal = _face_info->normal();
    Moose::FV::interpolate(_advected_interp_method,
                           face_rho,
                           _rho(&_face_info->elem()),
                           _rho(_face_info->neighborPtr()),
                           v,
                           *_face_info,
                           true);
  }
  return normal * v * face_rho;
}
