//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosExposedSideAverageValue.h"
#include "KokkosSelfShadowSideUserObject.h"

registerKokkosUserObject("HeatTransferApp", KokkosExposedSideAverageValue);

InputParameters
KokkosExposedSideAverageValue::validParams()
{
  InputParameters params = KokkosSideAverageValue::validParams();
  params.addClassDescription("Computes the average value of a variable on the "
                             "exposed portion of a sideset. Note that this cannot be used on the "
                             "centerline of an axisymmetric model.");
  params.addRequiredParam<UserObjectName>("self_shadow_uo",
                                          "KokkosSelfShadowSideUserObject that calculates the "
                                          "illumination state of element sides in a side set");
  return params;
}

KokkosExposedSideAverageValue::KokkosExposedSideAverageValue(const InputParameters & parameters)
  : KokkosSideAverageValue(parameters)
{
  const auto & self_shadow = getUserObject<KokkosSelfShadowSideUserObject>("self_shadow_uo");
  _illumination = self_shadow.illuminationStatus();

  if (self_shadow.useDisplacedMesh() != getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "The KokkosSelfShadowSideUserObject should operate on the same mesh (displaced or "
               "undisplaced) as this PostProcessor.");
}
