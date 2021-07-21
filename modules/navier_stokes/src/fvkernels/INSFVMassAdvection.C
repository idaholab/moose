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

  this->interpolate(_velocity_interp_method, v);
  const auto rho_interface =
      _rho(std::make_tuple(_face_info, _limiter.get(), v * _face_info->normal() > 0));
  return _normal * v * rho_interface;
}
