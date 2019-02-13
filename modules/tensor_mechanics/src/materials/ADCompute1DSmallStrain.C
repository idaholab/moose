//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCompute1DSmallStrain.h"

#include "libmesh/quadrature.h"

defineADValidParams(ADCompute1DSmallStrain,
                    ADComputeSmallStrain,
                    params.addClassDescription("Compute a small strain in 1D problem"););

template <ComputeStage compute_stage>
ADCompute1DSmallStrain<compute_stage>::ADCompute1DSmallStrain(const InputParameters & parameters)
  : ADComputeSmallStrain<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
void
ADCompute1DSmallStrain<compute_stage>::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    _total_strain[_qp](0, 0) = (*_grad_disp[0])[_qp](0);
    _total_strain[_qp](1, 1) = computeStrainYY();
    _total_strain[_qp](2, 2) = computeStrainZZ();

    _mechanical_strain[_qp] = _total_strain[_qp];

    // Remove the eigenstrain
    for (const auto es : _eigenstrains)
      _mechanical_strain[_qp] -= (*es)[_qp];
  }

  copyDualNumbersToValues();
}

// explicit instantiation is required for AD base classes
adBaseClass(ADCompute1DSmallStrain);
