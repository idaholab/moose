/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeSmallStrain.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<ComputeSmallStrain>()
{
  InputParameters params = validParams<ComputeStrainBase>();
  params.addClassDescription("Compute a small strain.");
  return params;
}

ComputeSmallStrain::ComputeSmallStrain(const InputParameters & parameters) :
    ComputeStrainBase(parameters)
{
}

void
ComputeSmallStrain::computeQpProperties()
{
  //strain = (grad_disp + grad_disp^T)/2
  RankTwoTensor grad_tensor((*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);

  _total_strain[_qp] = (grad_tensor + grad_tensor.transpose()) / 2.0;

  _mechanical_strain[_qp] = _total_strain[_qp];

  //Remove the eigenstrains
  for (auto es : _eigenstrains)
    _mechanical_strain[_qp] -= (*es)[_qp];
}
