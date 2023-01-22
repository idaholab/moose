//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DirectionalFluxBC.h"
#include "SelfShadowSideUserObject.h"

registerMooseObject("HeatConductionApp", DirectionalFluxBC);

InputParameters
DirectionalFluxBC::validParams()
{
  InputParameters params = FunctionNeumannBC::validParams();
  params.addClassDescription(
      "Applies a directional flux multiplied by the surface normal vector. "
      "Can utilize the self shadowing calculation from a SelfShadowSideUserObject.");
  params.addRequiredParam<RealVectorValue>("illumination_flux",
                                           "Radiation direction and magnitude vector");
  params.addParam<UserObjectName>(
      "self_shadow_uo",
      "SelfShadowSideUserObject that calculates the illumination state of a side set");
  params.set<FunctionName>("function") = "1";
  return params;
}

DirectionalFluxBC::DirectionalFluxBC(const InputParameters & parameters)
  : FunctionNeumannBC(parameters),
    _direction(getParam<RealVectorValue>("illumination_flux")),
    _self_shadow(isParamValid("self_shadow_uo")
                     ? &getUserObject<SelfShadowSideUserObject>("self_shadow_uo")
                     : nullptr)
{
  if (_self_shadow && _self_shadow->useDisplacedMesh() != getParam<bool>("use_displaced_mesh"))
    paramWarning("use_displaced_mesh",
                 "The SelfShadowSideUserObject should operate on the same mesh (displaced or "
                 "undisplaced) as this BC.");
}

void
DirectionalFluxBC::precalculateResidual()
{
  if (_self_shadow)
  {
    const SelfShadowSideUserObject::SideIDType id(_current_elem->id(), _current_side);
    _illumination = _self_shadow->illumination(id);
  }
  else
    // all bits set
    _illumination = ~0u;
}

Real
DirectionalFluxBC::computeQpResidual()
{
  auto projected_flux = -_direction * _normals[_qp];
  if (projected_flux > 0)
  {
    // tests if the bit at position _qp is set
    if (_illumination & (1 << _qp))
      return FunctionNeumannBC::computeQpResidual() * projected_flux;
  }
  return 0.0;
}
