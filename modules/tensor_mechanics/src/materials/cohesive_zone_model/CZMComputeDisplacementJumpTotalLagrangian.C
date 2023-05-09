//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMComputeDisplacementJumpTotalLagrangian.h"
#include "CohesiveZoneModelTools.h"
#include "FactorizedRankTwoTensor.h"

registerMooseObject("TensorMechanicsApp", CZMComputeDisplacementJumpTotalLagrangian);
registerMooseObject("TensorMechanicsApp", ADCZMComputeDisplacementJumpTotalLagrangian);

template <bool is_ad>
InputParameters
CZMComputeDisplacementJumpTotalLagrangianTempl<is_ad>::validParams()
{
  InputParameters params = CZMComputeDisplacementJumpBase<is_ad>::validParams();
  params.addClassDescription(
      "Compute the displacement jump increment across a czm interface in local "
      "coordinates for the Total Lagrangian kinematic formulation");

  return params;
}

template <bool is_ad>
CZMComputeDisplacementJumpTotalLagrangianTempl<
    is_ad>::CZMComputeDisplacementJumpTotalLagrangianTempl(const InputParameters & parameters)
  : CZMComputeDisplacementJumpBase<is_ad>(parameters),
    _F(this->template declareGenericPropertyByName<RankTwoTensor, is_ad>(_base_name + "F_czm")),
    _R(this->template declareGenericPropertyByName<RankTwoTensor, is_ad>(_base_name +
                                                                         "czm_rotation")),
    _czm_reference_rotation(this->template declareGenericPropertyByName<RankTwoTensor, is_ad>(
        _base_name + "czm_reference_rotation"))
{
  // initializing the displacement gradient vectors
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _grad_disp.push_back(&this->template coupledGenericGradient<is_ad>("displacements", i));
    _grad_disp_neighbor.push_back(
        &this->template coupledGenericNeighborGradient<is_ad>("displacements", i));
  }

  // All others zero (so this will work naturally for 2D and 1D problems)
  for (unsigned int i = _ndisp; i < 3; i++)
  {
    if constexpr (is_ad)
    {
      _grad_disp.push_back(&_ad_grad_zero);
      _grad_disp_neighbor.push_back(&_ad_grad_zero);
    }
    else
    {
      _grad_disp.push_back(&_grad_zero);
      _grad_disp_neighbor.push_back(&_grad_zero);
    }
  }
}

template <bool is_ad>
void
CZMComputeDisplacementJumpTotalLagrangianTempl<is_ad>::initQpStatefulProperties()
{
  CZMComputeDisplacementJumpBase<is_ad>::initQpStatefulProperties();
  _displacement_jump_global[_qp] = 0;
  _F[_qp].setToIdentity();
  _R[_qp].setToIdentity();
}

template <bool is_ad>
void
CZMComputeDisplacementJumpTotalLagrangianTempl<is_ad>::computeLocalDisplacementJump()
{
  _interface_displacement_jump[_qp] =
      _czm_total_rotation[_qp].transpose() * _displacement_jump_global[_qp];
}

template <bool is_ad>
void
CZMComputeDisplacementJumpTotalLagrangianTempl<is_ad>::computeRotationMatrices()
{
  _czm_reference_rotation[_qp] =
      CohesiveZoneModelTools::computeReferenceRotation(_normals[_qp], this->_mesh.dimension());
  computeFandR();
  _czm_total_rotation[_qp] = _R[_qp] * _czm_reference_rotation[_qp];
}

template <bool is_ad>
void
CZMComputeDisplacementJumpTotalLagrangianTempl<is_ad>::computeFandR()
{
  const auto F = (GenericRankTwoTensor<is_ad>::Identity() +
                  GenericRankTwoTensor<is_ad>::initializeFromRows(
                      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]));
  const auto F_neighbor =
      (GenericRankTwoTensor<is_ad>::Identity() +
       GenericRankTwoTensor<is_ad>::initializeFromRows((*_grad_disp_neighbor[0])[_qp],
                                                       (*_grad_disp_neighbor[1])[_qp],
                                                       (*_grad_disp_neighbor[2])[_qp]));

  _F[_qp] = 0.5 * (F + F_neighbor);
  // According to Cody mooseError are always fatal, so nothing we can do about
  // them. The norm of the tensor might work, but there is the risk of an
  // unwanted overflow (I tried and it happens). So checking component by
  // component remains the only reasonable strategy. If someone finds a  better
  // way this could be changed in the future
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      if (!std::isfinite(MetaPhysicL::raw_value(_F[_qp](i, j))))
        throw MooseException("CZMMaterialBaseIncremental _F is not finite, reducing time step");

  using FR2T = std::conditional_t<is_ad, ADFactorizedRankTwoTensor, FactorizedRankTwoTensor>;
  const FR2T C = _F[_qp].transpose() * _F[_qp];
  const auto Uinv = MathUtils::sqrt(C).inverse().get();
  _R[_qp] = _F[_qp] * Uinv;
}

template class CZMComputeDisplacementJumpTotalLagrangianTempl<false>;
template class CZMComputeDisplacementJumpTotalLagrangianTempl<true>;
