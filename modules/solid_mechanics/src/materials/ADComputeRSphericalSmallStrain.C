//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeRSphericalSmallStrain.h"
#include "Assembly.h"

#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", ADComputeRSphericalSmallStrain);

InputParameters
ADComputeRSphericalSmallStrain::validParams()
{
  InputParameters params = ADComputeSmallStrain::validParams();
  params.addClassDescription("Compute a small strain 1D spherical symmetry case.");
  return params;
}

ADComputeRSphericalSmallStrain::ADComputeRSphericalSmallStrain(const InputParameters & parameters)
  : ADComputeSmallStrain(parameters)
{
}

void
ADComputeRSphericalSmallStrain::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _total_strain[_qp](0, 0) = (*_grad_disp[0])[_qp](0);

    if (_q_point[_qp](0) != 0.0)
      _total_strain[_qp](1, 1) = (*_disp[0])[_qp] / _q_point[_qp](0);

    else
      _total_strain[_qp](1, 1) = 0.0;

    // \epsilon_{\theta \theta} = \epsilon{\phi \phi} in this 1D spherical system
    _total_strain[_qp](2, 2) = _total_strain[_qp](1, 1);

    _mechanical_strain[_qp] = _total_strain[_qp];

    // Remove the eigenstrains
    for (auto es : _eigenstrains)
      _mechanical_strain[_qp] -= (*es)[_qp];
  }
}
