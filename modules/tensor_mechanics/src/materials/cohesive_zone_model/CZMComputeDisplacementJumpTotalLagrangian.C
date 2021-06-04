//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMComputeDisplacementJumpTotalLagrangian.h"

registerMooseObject("TensorMechanicsApp", CZMComputeDisplacementJumpTotalLagrangian);

InputParameters
CZMComputeDisplacementJumpTotalLagrangian::validParams()
{
  InputParameters params = CZMComputeDisplacementJumpSmallStrain::validParams();
  params.addClassDescription(
      "Compute the displacement jump increment accross a czm interface in local "
      "coordinates for the Total Lagrangian kinematic formulation");

  return params;
}

CZMComputeDisplacementJumpTotalLagrangian::CZMComputeDisplacementJumpTotalLagrangian(
    const InputParameters & parameters)
  : CZMComputeDisplacementJumpSmallStrain(parameters),
    _displacement_jump_global_old(
        getMaterialPropertyOld<RealVectorValue>("displacement_jump_global")),
    _interface_displacement_jump_old(
        getMaterialPropertyOld<RealVectorValue>("interface_displacement_jump")),
    _displacement_jump_global_inc(declareProperty<RealVectorValue>("displacement_jump_global_inc")),
    _interface_displacement_jump_inc(
        declareProperty<RealVectorValue>("interface_displacement_jump_inc")),
    _F(declareProperty<RankTwoTensor>("F_czm")),
    _F_old(getMaterialPropertyOld<RankTwoTensor>("F_czm")),
    _R(declareProperty<RankTwoTensor>("czm_rotation")),
    _R_old(getMaterialPropertyOld<RankTwoTensor>("czm_rotation")),
    _Q(declareProperty<RankTwoTensor>("czm_total_rotation")),
    _DQ(declareProperty<RankTwoTensor>("czm_total_rotation_inc"))
{
  // Enforce consistency
  if (_ndisp != _mesh.dimension())
    paramError("displacements", "Number of displacements must match problem dimension.");

  if (_ndisp > 3 || _ndisp < 1)
    mooseError("the CZM material requires 1, 2 or 3 displacement variables");
}

void
CZMComputeDisplacementJumpTotalLagrangian::initialSetup()
{
  ///intialize displacementes vectors
  CZMComputeDisplacementJumpSmallStrain::initialSetup();

  // initializing the displacement gradeint vectors
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
  CZMComputeDisplacementJumpSmallStrain::initQpStatefulProperties();
  _displacement_jump_global[_qp] = 0;
  _F[_qp] = RankTwoTensor::Identity();
  _R[_qp] = RankTwoTensor::Identity();
}

void
CZMComputeDisplacementJumpTotalLagrangian::computeLocalDisplacementJump()
{
  computeFandR();
  _DQ[_qp] = (_R[_qp] - _R_old[_qp]) * _Q0[_qp];
  _Q[_qp] = _R[_qp] * _Q0[_qp];
  _displacement_jump_global_inc[_qp] =
      _displacement_jump_global[_qp] - _displacement_jump_global_old[_qp];

  _interface_displacement_jump_inc[_qp] = _DQ[_qp].transpose() * _displacement_jump_global[_qp] +
                                          _Q[_qp].transpose() * _displacement_jump_global_inc[_qp];
  _interface_displacement_jump[_qp] =
      _interface_displacement_jump_old[_qp] + _interface_displacement_jump_inc[_qp];
}

void
CZMComputeDisplacementJumpTotalLagrangian::computeFandR()
{
  RankTwoTensor F =
      (RankTwoTensor::Identity() +
       RankTwoTensor((*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]));
  RankTwoTensor F_neighbor =
      (RankTwoTensor::Identity() + RankTwoTensor((*_grad_disp_neighbor[0])[_qp],
                                                 (*_grad_disp_neighbor[1])[_qp],
                                                 (*_grad_disp_neighbor[2])[_qp]));

  _F[_qp] = 0.5 * (F + F_neighbor);
  // According to Cody mooseError are always fatal, so nothing we can do about
  // them. The norm of the tensor might work, but there is the risk of an
  // unwanted overflow (I tried and it happens). So checking component by
  // component remains the only reasonable strategy. If someone finds a  better
  // way this could be changed in the future
  for (uint i = 0; i < 3; i++)
    for (uint j = 0; j < 3; j++)
      if (!std::isfinite(_F[_qp](i, j)))
        throw MooseException("CZMMaterialBaseIncremental _F is not finite, reducing time step");
  _F[_qp].getRUDecompositionRotation(_R[_qp]);
}
