/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeRSphericalSmallStrain.h"
#include "Assembly.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ComputeRSphericalSmallStrain>()
{
  InputParameters params = validParams<ComputeSmallStrain>();
  params.addClassDescription("Compute a small strain 1D spherical symmetry case.");
  return params;
}

ComputeRSphericalSmallStrain::ComputeRSphericalSmallStrain(const InputParameters & parameters)
  : ComputeSmallStrain(parameters)
{
}

void
ComputeRSphericalSmallStrain::computeProperties()
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
