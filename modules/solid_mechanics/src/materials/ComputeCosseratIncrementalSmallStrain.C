//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeCosseratIncrementalSmallStrain.h"

// MOOSE includes
#include "PermutationTensor.h"

#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", ComputeCosseratIncrementalSmallStrain);

InputParameters
ComputeCosseratIncrementalSmallStrain::validParams()
{
  InputParameters params = ComputeIncrementalStrainBase::validParams();
  params.addClassDescription("Compute incremental small Cosserat strains");
  params.addRequiredCoupledVar("Cosserat_rotations", "The 3 Cosserat rotation variables");
  return params;
}

ComputeCosseratIncrementalSmallStrain::ComputeCosseratIncrementalSmallStrain(
    const InputParameters & parameters)
  : ComputeIncrementalStrainBase(parameters),
    _curvature(declareProperty<RankTwoTensor>("curvature")),
    _nrots(coupledComponents("Cosserat_rotations")),
    _wc(coupledValues("Cosserat_rotations")),
    _wc_old(coupledValuesOld("Cosserat_rotations")),
    _grad_wc(coupledGradients("Cosserat_rotations")),
    _grad_wc_old(coupledGradientsOld("Cosserat_rotations")),
    _curvature_old(getMaterialPropertyOld<RankTwoTensor>("curvature")),
    _curvature_increment(declareProperty<RankTwoTensor>("curvature_increment"))
{
  if (_nrots != 3)
    mooseError("ComputeCosseratSmallStrain: This Material is only defined for 3-dimensional "
               "simulations so 3 Cosserat rotation variables are needed");
}

void
ComputeCosseratIncrementalSmallStrain::initQpStatefulProperties()
{
  ComputeIncrementalStrainBase::initQpStatefulProperties();

  _curvature[_qp].zero();
}

void
ComputeCosseratIncrementalSmallStrain::computeQpProperties()
{
  auto strain = RankTwoTensor::initializeFromRows(
      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);
  auto strain_old = RankTwoTensor::initializeFromRows(
      (*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]);
  RealVectorValue wc_vector((*_wc[0])[_qp], (*_wc[1])[_qp], (*_wc[2])[_qp]);
  RealVectorValue wc_vector_old((*_wc_old[0])[_qp], (*_wc_old[1])[_qp], (*_wc_old[2])[_qp]);

  for (unsigned i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned k = 0; k < LIBMESH_DIM; ++k)
      {
        strain(i, j) += PermutationTensor::eps(i, j, k) * wc_vector(k);
        strain_old(i, j) += PermutationTensor::eps(i, j, k) * wc_vector_old(k);
      }

  // Gauss point deformation gradient
  _deformation_gradient[_qp] = strain;
  _deformation_gradient[_qp].addIa(1.0);

  const RankTwoTensor total_strain_increment = strain - strain_old;

  _strain_increment[_qp] = total_strain_increment;

  // Remove the eigenstrain increment
  subtractEigenstrainIncrementFromStrain(_strain_increment[_qp]);

  _strain_rate[_qp] = _strain_increment[_qp] / _dt;

  _total_strain[_qp] = _total_strain_old[_qp] + total_strain_increment;
  _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];

  auto curv = RankTwoTensor::initializeFromRows(
      (*_grad_wc[0])[_qp], (*_grad_wc[1])[_qp], (*_grad_wc[2])[_qp]);
  auto curv_old = RankTwoTensor::initializeFromRows(
      (*_grad_wc_old[0])[_qp], (*_grad_wc_old[1])[_qp], (*_grad_wc_old[2])[_qp]);
  _curvature_increment[_qp] = curv - curv_old;
  _curvature[_qp] = _curvature_old[_qp] + _curvature_increment[_qp];

  // incremental small strain does not include rotation
  _rotation_increment[_qp].setToIdentity();
}
