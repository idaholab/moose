//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AreaCorrectedSideIntegralMaterialProperty.h"

#include "metaphysicl/raw_type.h"

#include "BoundaryShortestDistanceToSurface.h"

registerMooseObject("ShiftedBoundaryMethodApp", AreaCorrectedSideIntegralMaterialProperty);
registerMooseObject("ShiftedBoundaryMethodApp", ADAreaCorrectedSideIntegralMaterialProperty);

template <bool is_ad>
InputParameters
AreaCorrectedSideIntegralMaterialPropertyTempl<is_ad>::validParams()
{
  InputParameters params = SideIntegralMaterialPropertyTempl<is_ad>::validParams();
  params.addParam<UserObjectName>(
      "sbm_distance_uo",
      "UserObject that provides signed distance and normal vector calculations.");
  params.addParam<bool>("no_shifted", false, "Disable area correction terms.");

  params.addClassDescription(
      "Compute the integral of a scalar material property component over "
      "the surrogate boundary with the area correction term (n dot Tilde{n}).");
  return params;
}

template <bool is_ad>
AreaCorrectedSideIntegralMaterialPropertyTempl<
    is_ad>::AreaCorrectedSideIntegralMaterialPropertyTempl(const InputParameters & parameters)
  : SideIntegralMaterialPropertyTempl<is_ad>(parameters),
    _shifted(!this->template getParam<bool>("no_shifted"))
{
}

template <bool is_ad>
void
AreaCorrectedSideIntegralMaterialPropertyTempl<is_ad>::initialSetup()
{
  if (_shifted)
  {
    if (!this->isParamSetByUser("sbm_distance_uo"))
      mooseError("Please provide 'sbm_distance_uo' when using shifted area correction.");

    _sbm_distance_uo =
        &this->template getUserObject<BoundaryShortestDistanceToSurface>("sbm_distance_uo");
  }
}

template <bool is_ad>
Real
AreaCorrectedSideIntegralMaterialPropertyTempl<is_ad>::computeQpIntegral()
{
  if (!_shifted)
    return SideIntegralMaterialPropertyTempl<is_ad>::computeQpIntegral();

  const auto elem_side = std::make_pair(this->_current_elem->id(), this->_current_side);
  const auto true_normal = _sbm_distance_uo->trueNormal(elem_side, this->_qp);
  const auto true_dot_surrogate_normal = true_normal * this->_normals[this->_qp];

  return SideIntegralMaterialPropertyTempl<is_ad>::computeQpIntegral() * true_dot_surrogate_normal;
}

template class AreaCorrectedSideIntegralMaterialPropertyTempl<false>;
template class AreaCorrectedSideIntegralMaterialPropertyTempl<true>;
