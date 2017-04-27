/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeCosseratSmallStrain.h"

// MOOSE includes
#include "RankThreeTensor.h"

// libMesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ComputeCosseratSmallStrain>()
{
  InputParameters params = validParams<ComputeStrainBase>();
  params.addClassDescription("Compute small Cosserat strains");
  params.addRequiredCoupledVar("Cosserat_rotations", "The 3 Cosserat rotation variables");
  return params;
}

ComputeCosseratSmallStrain::ComputeCosseratSmallStrain(const InputParameters & parameters)
  : ComputeStrainBase(parameters),
    _curvature(declareProperty<RankTwoTensor>("curvature")),
    _nrots(coupledComponents("Cosserat_rotations")),
    _wc(_nrots),
    _grad_wc(_nrots)
{
  if (_nrots != 3)
    mooseError("ComputeCosseratSmallStrain: This Material is only defined for 3-dimensional "
               "simulations so 3 Cosserat rotation variables are needed");
  for (unsigned i = 0; i < _nrots; ++i)
  {
    _wc[i] = &coupledValue("Cosserat_rotations", i);
    _grad_wc[i] = &coupledGradient("Cosserat_rotations", i);
  }
}

void
ComputeCosseratSmallStrain::computeQpProperties()
{
  RankTwoTensor strain((*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);
  RealVectorValue wc_vector((*_wc[0])[_qp], (*_wc[1])[_qp], (*_wc[2])[_qp]);

  for (unsigned i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned k = 0; k < LIBMESH_DIM; ++k)
        strain(i, j) += RankThreeTensor::leviCivita(i, j, k) * wc_vector(k);

  _total_strain[_qp] = strain;

  _mechanical_strain[_qp] = strain;
  for (auto es : _eigenstrains)
    _mechanical_strain[_qp] -= (*es)[_qp];

  _curvature[_qp] = RankTwoTensor((*_grad_wc[0])[_qp], (*_grad_wc[1])[_qp], (*_grad_wc[2])[_qp]);
}
