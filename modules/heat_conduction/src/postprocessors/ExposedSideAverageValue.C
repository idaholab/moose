//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExposedSideAverageValue.h"
#include "SelfShadowSideUserObject.h"

registerMooseObject("HeatConductionApp", ExposedSideAverageValue);

InputParameters
ExposedSideAverageValue::validParams()
{
  InputParameters params = SideAverageValue::validParams();
  params.addClassDescription("Computes the average value of a variable on the "
                             "exposed portion of a sideset. Note that this cannot be used on the "
                             "centerline of an axisymmetric model.");
  params.addRequiredParam<UserObjectName>("self_shadow_uo",
                                          "SelfShadowSideUserObject that calculates the "
                                          "illumination state of element sides in a side set");
  return params;
}

ExposedSideAverageValue::ExposedSideAverageValue(const InputParameters & parameters)
  : SideAverageValue(parameters),
    _self_shadow(getUserObject<SelfShadowSideUserObject>("self_shadow_uo"))
{
  if (_self_shadow.useDisplacedMesh() != getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "The SelfShadowSideUserObject should operate on the same mesh (displaced or "
               "undisplaced) as this PostProcessor.");
}

Real
ExposedSideAverageValue::computeQpIntegral()
{
  const SelfShadowSideUserObject::SideIDType id(_current_elem->id(), _current_side);
  const unsigned int illumination = _self_shadow.illumination(id);
  // tests if the bit at position _qp is set
  if (illumination & (1 << _qp))
    return SideAverageValue::computeQpIntegral();
  else
    return 0.0;
}

Real
ExposedSideAverageValue::volume()
{
  Real curr_exposed_side_volume = 0.0;
  const SelfShadowSideUserObject::SideIDType id(_current_elem->id(), _current_side);
  const unsigned int illumination = _self_shadow.illumination(id);
  for (const unsigned int qp : make_range(_qrule->n_points()))
  {
    // tests if the bit at position _qp is set
    if (illumination & (1 << qp))
      curr_exposed_side_volume += _JxW[qp] * _coord[qp];
  }
  return curr_exposed_side_volume;
}
