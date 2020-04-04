//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Compute1DSmallStrain.h"

#include "libmesh/quadrature.h"

InputParameters
Compute1DSmallStrain::validParams()
{
  InputParameters params = ComputeSmallStrain::validParams();
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
