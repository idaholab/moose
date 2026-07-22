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
  InputParameters params = InterfaceMaterial::validParams();
  params.addClassDescription("Base class used to compute the displacement jump across a czm "
                             "interface in local coordinates");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.suppressParameter<bool>("use_displaced_mesh");

  // no shifted testing
  params.addParam<bool>("no_shifted", false, "Disable shifted terms.");

  params.addParam<UserObjectName>(
      "sbm_distance_uo",
      "UserObject that provides signed distance and normal vector calculations.");

  params.addParam<std::string>("base_name", "Material property base name");

  return params;
}

template <bool is_ad>
SCZMComputeDisplacementJumpBase<is_ad>::SCZMComputeDisplacementJumpBase(
    const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name") + "_"
                   : ""),
    _ndisp(coupledComponents("displacements")),
    _disp(3),
    _disp_neighbor(3),
    _grad_disp(3),
    _grad_disp_neighbor(3),
    _displacement_jump_global(declareGenericPropertyByName<RealVectorValue, is_ad>(
        _base_name + "displacement_jump_global")),
    _interface_displacement_jump(declareGenericPropertyByName<RealVectorValue, is_ad>(
        _base_name + "interface_displacement_jump")),
    _czm_total_rotation(
        declareGenericPropertyByName<RankTwoTensor, is_ad>(_base_name + "czm_total_rotation")),
    _shifted(!getParam<bool>("no_shifted")),
    _sbm_distance_uo(nullptr)
{
  // Enforce consistency
  if (_ndisp != _mesh.dimension())
    paramError("displacements", "Number of displacements must match problem dimension.");

  if (_ndisp > 3 || _ndisp < 1)
    mooseError("the CZM material requires 1, 2 or 3 displacement variables");

  // initializing the displacement vectors
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp[i] = &coupledGenericValue<is_ad>("displacements", i);
    _disp_neighbor[i] = &coupledGenericNeighborValue<is_ad>("displacements", i);
    _grad_disp[i] = &coupledGenericGradient<is_ad>("displacements", i);
    _grad_disp_neighbor[i] = &coupledGenericNeighborGradient<is_ad>("displacements", i);
  }

  // All others zero (so this will work naturally for 2D and 1D problems)
  for (unsigned int i = _ndisp; i < 3; i++)
  {
    if constexpr (is_ad)
    {
      _disp[i] = &_ad_zero;
      _disp_neighbor[i] = &_ad_zero;
      _grad_disp[i] = &_ad_grad_zero;
      _grad_disp_neighbor[i] = &_ad_grad_zero;
    }
    else
    {
      _disp[i] = &_zero;
      _disp_neighbor[i] = &_zero;
      _grad_disp[i] = &_grad_zero;
      _grad_disp_neighbor[i] = &_grad_zero;
    }
  }
}

template <bool is_ad>
void
SCZMComputeDisplacementJumpBase<is_ad>::initialSetup()
{
  if (_shifted)
  {
    if (!isParamSetByUser("sbm_distance_uo"))
      mooseError("Please provide 'sbm_distance_uo' when using shifted terms.");
    _sbm_distance_uo = &getUserObject<BoundaryShortestDistanceToSurface>("sbm_distance_uo");
  }
}

template <bool is_ad>
void
SCZMComputeDisplacementJumpBase<is_ad>::initQpStatefulProperties()
{
  // requried to promote _interface_displacement_jump to stateful in case someone needs it
  _interface_displacement_jump[_qp] = 0;
}

template <bool is_ad>
void
SCZMComputeDisplacementJumpBase<is_ad>::computeQpProperties()
{

  // computing the displacement jump
  for (unsigned int i = 0; i < _ndisp; i++)
    _displacement_jump_global[_qp](i) = (*_disp_neighbor[i])[_qp] - (*_disp[i])[_qp];
  for (unsigned int i = _ndisp; i < 3; i++)
    _displacement_jump_global[_qp](i) = 0;

  if (_shifted)
  {
    const auto d = _sbm_distance_uo->surrogateDistance(
        std::make_pair(_current_elem->id(), _current_side), _qp);
    for (unsigned int i = 0; i < _ndisp; i++)
      _displacement_jump_global[_qp](i) +=
          ((*_grad_disp_neighbor[i])[_qp] - (*_grad_disp[i])[_qp]) * d;
  }

  computeRotationMatrices();
  computeLocalDisplacementJump();
}

template <bool is_ad>
void
SCZMComputeDisplacementJumpBase<is_ad>::computeRotationMatrices()
{
  const auto true_normal = _shifted ? RealVectorValue(_sbm_distance_uo->trueNormal(
                                          std::make_pair(_current_elem->id(), _current_side), _qp))
                                    : RealVectorValue(_normals[_qp]);

  _czm_total_rotation[_qp] =
      CohesiveZoneModelTools::computeReferenceRotation<is_ad>(true_normal, _mesh.dimension());
}

template class SCZMComputeDisplacementJumpBase<false>;
template class SCZMComputeDisplacementJumpBase<true>;
