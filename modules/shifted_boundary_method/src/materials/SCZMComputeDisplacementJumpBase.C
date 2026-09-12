//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCZMComputeDisplacementJumpBase.h"
#include "CohesiveZoneModelTools.h"
#include "BoundaryShortestDistanceToSurface.h"

template <bool is_ad>
InputParameters
SCZMComputeDisplacementJumpBase<is_ad>::validParams()
{
  InputParameters params = CZMComputeDisplacementJumpBase<is_ad>::validParams();
  params.addClassDescription(
      "Base class used to compute the shifted displacement jump across a czm "
      "interface in local coordinates");

  // no shifted testing
  params.addParam<bool>("no_shifted", false, "Disable shifted terms.");

  params.addParam<UserObjectName>(
      "sbm_distance_uo",
      "UserObject that provides signed distance and normal vector calculations.");

  return params;
}

template <bool is_ad>
SCZMComputeDisplacementJumpBase<is_ad>::SCZMComputeDisplacementJumpBase(
    const InputParameters & parameters)
  : CZMComputeDisplacementJumpBase<is_ad>(parameters),
    _grad_disp(3),
    _grad_disp_neighbor(3),
    _shifted(!this->template getParam<bool>("no_shifted")),
    _sbm_distance_uo(nullptr)
{
  for (const auto i : make_range(_ndisp))
  {
    _grad_disp[i] = &this->template coupledGenericGradient<is_ad>("displacements", i);
    _grad_disp_neighbor[i] =
        &this->template coupledGenericNeighborGradient<is_ad>("displacements", i);
  }

  for (const auto i : make_range(_ndisp, 3u))
    if constexpr (is_ad)
    {
      _grad_disp[i] = &this->_ad_grad_zero;
      _grad_disp_neighbor[i] = &this->_ad_grad_zero;
    }
    else
    {
      _grad_disp[i] = &this->_grad_zero;
      _grad_disp_neighbor[i] = &this->_grad_zero;
    }
}

template <bool is_ad>
void
SCZMComputeDisplacementJumpBase<is_ad>::initialSetup()
{
  if (_shifted)
  {
    if (!this->isParamSetByUser("sbm_distance_uo"))
      mooseError("Please provide 'sbm_distance_uo' when using shifted terms.");
    _sbm_distance_uo =
        &this->template getUserObject<BoundaryShortestDistanceToSurface>("sbm_distance_uo");
  }
}

template <bool is_ad>
void
SCZMComputeDisplacementJumpBase<is_ad>::computeGlobalDisplacementJump()
{
  CZMComputeDisplacementJumpBase<is_ad>::computeGlobalDisplacementJump();

  if (_shifted)
  {
    const auto d = _sbm_distance_uo->surrogateDistance(
        std::make_pair(this->_current_elem->id(), this->_current_side), _qp);
    for (const auto i : make_range(_ndisp))
      _displacement_jump_global[_qp](i) +=
          ((*_grad_disp_neighbor[i])[_qp] - (*_grad_disp[i])[_qp]) * d;
  }
}

template <bool is_ad>
void
SCZMComputeDisplacementJumpBase<is_ad>::computeRotationMatrices()
{
  const auto true_normal =
      _shifted ? RealVectorValue(_sbm_distance_uo->trueNormal(
                     std::make_pair(this->_current_elem->id(), this->_current_side), _qp))
               : RealVectorValue(this->_normals[_qp]);

  _czm_total_rotation[_qp] =
      CohesiveZoneModelTools::computeReferenceRotation<is_ad>(true_normal, _mesh.dimension());
}

template class SCZMComputeDisplacementJumpBase<false>;
template class SCZMComputeDisplacementJumpBase<true>;
