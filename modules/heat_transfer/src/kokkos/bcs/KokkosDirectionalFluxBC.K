//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosDirectionalFluxBC.h"

registerKokkosResidualObject("HeatTransferApp", KokkosDirectionalFluxBC);

InputParameters
KokkosDirectionalFluxBC::validParams()
{
  InputParameters params = IntegratedBCValue::validParams();
  params.addClassDescription(
      "Applies a directional flux multiplied by the surface normal vector. "
      "Can utilize the self shadowing calculation from a KokkosSelfShadowSideUserObject.");
  params.addRequiredParam<RealVectorValue>("illumination_flux",
                                           "Radiation direction and magnitude vector");
  params.addParam<UserObjectName>(
      "self_shadow_uo",
      "KokkosSelfShadowSideUserObject that calculates the illumination state of a side set");
  params.addParam<FunctionName>("function", "1", "The function multiplying the flux.");
  return params;
}

KokkosDirectionalFluxBC::KokkosDirectionalFluxBC(const InputParameters & parameters)
  : IntegratedBCValue(parameters),
    _direction(getParam<RealVectorValue>("illumination_flux")),
    _func(getKokkosFunction("function")),
    _has_self_shadow(isParamValid("self_shadow_uo"))
{
  if (!_has_self_shadow)
    return;

  const auto & self_shadow = getUserObject<KokkosSelfShadowSideUserObject>("self_shadow_uo");
  _illumination = self_shadow.illuminationStatus();

  if (self_shadow.useDisplacedMesh() != getParam<bool>("use_displaced_mesh"))
    paramWarning("use_displaced_mesh",
                 "The KokkosSelfShadowSideUserObject should operate on the same mesh (displaced or "
                 "undisplaced) as this BC.");
}
