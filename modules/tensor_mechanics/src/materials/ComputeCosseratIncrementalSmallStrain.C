/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeCosseratIncrementalSmallStrain.h"

// MOOSE includes
#include "RankThreeTensor.h"

// libMesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ComputeCosseratIncrementalSmallStrain>()
{
  InputParameters params = validParams<ComputeIncrementalStrainBase>();
  params.addClassDescription("Compute incremental small Cosserat strains");
  params.addRequiredCoupledVar("Cosserat_rotations", "The 3 Cosserat rotation variables");
  return params;
}

ComputeCosseratIncrementalSmallStrain::ComputeCosseratIncrementalSmallStrain(
    const InputParameters & parameters)
  : ComputeIncrementalStrainBase(parameters),
    _curvature(declareProperty<RankTwoTensor>("curvature")),
    _nrots(coupledComponents("Cosserat_rotations")),
    _wc(_nrots),
    _wc_old(_nrots),
    _grad_wc(_nrots),
    _grad_wc_old(_nrots),
    _curvature_old(declarePropertyOld<RankTwoTensor>("curvature")),
    _curvature_increment(declareProperty<RankTwoTensor>("curvature_increment"))
{
  if (_nrots != 3)
    mooseError("ComputeCosseratSmallStrain: This Material is only defined for 3-dimensional "
               "simulations so 3 Cosserat rotation variables are needed");
  for (unsigned i = 0; i < _nrots; ++i)
  {
    _wc[i] = &coupledValue("Cosserat_rotations", i);
    _wc_old[i] = &coupledValueOld("Cosserat_rotations", i);
    _grad_wc[i] = &coupledGradient("Cosserat_rotations", i);
    _grad_wc_old[i] = &coupledGradientOld("Cosserat_rotations", i);
  }
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
  RankTwoTensor strain((*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);
  RankTwoTensor strain_old(
      (*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]);
  RealVectorValue wc_vector((*_wc[0])[_qp], (*_wc[1])[_qp], (*_wc[2])[_qp]);
  RealVectorValue wc_vector_old((*_wc_old[0])[_qp], (*_wc_old[1])[_qp], (*_wc_old[2])[_qp]);

  for (unsigned i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned k = 0; k < LIBMESH_DIM; ++k)
      {
        strain(i, j) += RankThreeTensor::leviCivita(i, j, k) * wc_vector(k);
        strain_old(i, j) += RankThreeTensor::leviCivita(i, j, k) * wc_vector_old(k);
      }

  _deformation_gradient[_qp] = strain;
  _deformation_gradient[_qp].addIa(1.0); // Gauss point deformation gradient

  const RankTwoTensor total_strain_increment = strain - strain_old;

  _strain_increment[_qp] = total_strain_increment;

  // Remove the eigenstrain increment
  subtractEigenstrainIncrementFromStrain(_strain_increment[_qp]);

  _strain_rate[_qp] = _strain_increment[_qp] / _dt;

  _total_strain[_qp] = _total_strain_old[_qp] + total_strain_increment;
  _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];

  RankTwoTensor curv((*_grad_wc[0])[_qp], (*_grad_wc[1])[_qp], (*_grad_wc[2])[_qp]);
  RankTwoTensor curv_old((*_grad_wc_old[0])[_qp], (*_grad_wc_old[1])[_qp], (*_grad_wc_old[2])[_qp]);
  _curvature_increment[_qp] = curv - curv_old;
  _curvature[_qp] = _curvature_old[_qp] + _curvature_increment[_qp];
}
