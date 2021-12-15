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

registerMooseObject("TensorMechanicsApp", CZMComputeDisplacementJumpTotalLagrangian);

InputParameters
CZMComputeDisplacementJumpTotalLagrangian::validParams()
{
  InputParameters params = CZMComputeDisplacementJumpBase::validParams();
  params.addClassDescription(
      "Compute the displacement jump increment across a czm interface in local "
      "coordinates for the Total Lagrangian kinematic formulation");

  return params;
}

CZMComputeDisplacementJumpTotalLagrangian::CZMComputeDisplacementJumpTotalLagrangian(
    const InputParameters & parameters)
  : CZMComputeDisplacementJumpBase(parameters),
    _F(declarePropertyByName<RankTwoTensor>(_base_name + "F_czm")),
    _R(declarePropertyByName<RankTwoTensor>(_base_name + "czm_rotation")),
    _czm_reference_rotation(
        declarePropertyByName<RankTwoTensor>(_base_name + "czm_reference_rotation"))
{
  // initializing the displacement gradient vectors
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _grad_disp.push_back(&coupledGradient("displacements", i));
    _grad_disp_neighbor.push_back(&coupledNeighborGradient("displacements", i));
  }

  // All others zero (so this will work naturally for 2D and 1D problems)
  for (unsigned int i = _ndisp; i < 3; i++)
  {
    _grad_disp.push_back(&_grad_zero);
    _grad_disp_neighbor.push_back(&_grad_zero);
  }
}

void
CZMComputeDisplacementJumpTotalLagrangian::initQpStatefulProperties()
{
  CZMComputeDisplacementJumpBase::initQpStatefulProperties();
  _displacement_jump_global[_qp] = 0;
  _F[_qp] = RankTwoTensor::Identity();
  _R[_qp] = RankTwoTensor::Identity();
}

void
CZMComputeDisplacementJumpTotalLagrangian::computeLocalDisplacementJump()
{
  _interface_displacement_jump[_qp] =
      _czm_total_rotation[_qp].transpose() * _displacement_jump_global[_qp];
}

void
CZMComputeDisplacementJumpTotalLagrangian::computeRotationMatrices()
{

  _czm_reference_rotation[_qp] =
      CohesiveZoneModelTools::computeReferenceRotation(_normals[_qp], _mesh.dimension());
  computeFandR();
  _czm_total_rotation[_qp] = _R[_qp] * _czm_reference_rotation[_qp];
}

void
CZMComputeDisplacementJumpTotalLagrangian::computeFandR()
{
  RankTwoTensor F = (RankTwoTensor::Identity() +
                     RankTwoTensor::initializeFromRows(
                         (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]));
  RankTwoTensor F_neighbor = (RankTwoTensor::Identity() +
                              RankTwoTensor::initializeFromRows((*_grad_disp_neighbor[0])[_qp],
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
      if (!std::isfinite(_F[_qp](i, j)))
        throw MooseException("CZMMaterialBaseIncremental _F is not finite, reducing time step");
  _F[_qp].getRUDecompositionRotation(_R[_qp]);
}
