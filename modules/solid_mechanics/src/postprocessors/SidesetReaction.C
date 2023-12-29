//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SidesetReaction.h"
#include "metaphysicl/raw_type.h"

registerMooseObject("TensorMechanicsApp", SidesetReaction);
registerMooseObject("TensorMechanicsApp", ADSidesetReaction);

template <bool is_ad>
InputParameters
SidesetReactionTempl<is_ad>::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addClassDescription("Computes the integrated reaction force in a user-specified direction "
                             "on a sideset from the surface traction");
  params.addRequiredParam<MaterialPropertyName>("stress_tensor", "The rank two stress tensor name");
  params.addRequiredParam<RealVectorValue>("direction",
                                           "Direction in which the force is to be computed");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

template <bool is_ad>
SidesetReactionTempl<is_ad>::SidesetReactionTempl(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _tensor(getGenericMaterialProperty<RankTwoTensor, is_ad>("stress_tensor")),
    _dir(getParam<RealVectorValue>("direction"))
{
}

template <bool is_ad>
Real
SidesetReactionTempl<is_ad>::computeQpIntegral()
{
  return _normals[_qp] * (MetaPhysicL::raw_value(_tensor[_qp]) * _dir);
}

template class SidesetReactionTempl<false>;
template class SidesetReactionTempl<true>;
