//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMassAdvection.h"

registerMooseObject("NavierStokesApp", PINSFVMassAdvection);

InputParameters
PINSFVMassAdvection::validParams()
{
  auto params = INSFVMomentumAdvection::validParams();
  params.addClassDescription("Object for advecting mass, e.g. eps * rho");
  params.suppressParameter<MaterialPropertyName>("advected_quantity");
  return params;
}

PINSFVMassAdvection::PINSFVMassAdvection(const InputParameters & params)
  : INSFVMomentumAdvection(params),
  _eps(adCoupledValue("porosity")),
  _eps_neighbor(adCoupledNeighborValue("porosity"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
}

ADReal
PINSFVMassAdvection::computeQpResidual()
{
  /// Interpolate the velocity on the face
  ADRealVectorValue v;
  this->interpolate(_velocity_interp_method, v, _vel_elem[_qp], _vel_neighbor[_qp]);

  /// Interpolate the porosity on the face
  ADReal eps_face;
  Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
              eps_face,
              _eps[_qp],
              _eps_neighbor[_qp],
              *_face_info,
              true);

  return _normal * v * eps_face * _rho;
}
