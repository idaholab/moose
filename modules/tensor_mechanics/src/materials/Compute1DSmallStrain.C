/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Compute1DSmallStrain.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<Compute1DSmallStrain>()
{
  InputParameters params = validParams<ComputeSmallStrain>();
  params.addClassDescription("Compute a small strain in 1D problem");
  return params;
}

Compute1DSmallStrain::Compute1DSmallStrain(const InputParameters & parameters)
  : ComputeSmallStrain(parameters)
{
}

void
Compute1DSmallStrain::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _total_strain[_qp](0, 0) = (*_grad_disp[0])[_qp](0);
    _total_strain[_qp](1, 1) = computeStrainYY();
    _total_strain[_qp](2, 2) = computeStrainZZ();

    _mechanical_strain[_qp] = _total_strain[_qp];

    // Remove the eigenstrain
    for (auto es : _eigenstrains)
      _mechanical_strain[_qp] -= (*es)[_qp];
  }
}
