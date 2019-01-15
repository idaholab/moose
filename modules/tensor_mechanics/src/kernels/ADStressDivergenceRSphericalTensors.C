//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADStressDivergenceRSphericalTensors.h"
#include "ElasticityTensorTools.h"
#include "FEProblem.h"
#include "MooseMesh.h"

registerADMooseObject("TensorMechanicsApp", ADStressDivergenceRSphericalTensors);

defineADValidParams(
    ADStressDivergenceRSphericalTensors,
    ADStressDivergenceTensors,
    params.addClassDescription(
        "Calculate stress divergence for a spherically symmetric 1D problem in polar coordinates.");
    params.set<unsigned int>("component") = 0;
    return params;);

template <ComputeStage compute_stage>
ADStressDivergenceRSphericalTensors<compute_stage>::ADStressDivergenceRSphericalTensors(
    const InputParameters & parameters)
  : ADStressDivergenceTensors<compute_stage>(parameters)
{
  if (_component != 0)
    mooseError("Invalid component for this 1D RSpherical problem.");
}

template <ComputeStage compute_stage>
void
ADStressDivergenceRSphericalTensors<compute_stage>::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_RSPHERICAL)
    mooseError("The coordinate system in the Problem block must be set to RSPHERICAL for 1D "
               "spherically symmetric geometries.");
}

template <ComputeStage compute_stage>
ADResidual
ADStressDivergenceRSphericalTensors<compute_stage>::computeQpResidual()
{
  return _grad_test[_i][_qp](0) * _stress[_qp](0, 0) +               // stress_{rr} part 1
         +(_test[_i][_qp] / _q_point[_qp](0)) * _stress[_qp](1, 1) + // stress_{\theta \theta}
         +(_test[_i][_qp] / _q_point[_qp](0)) * _stress[_qp](2, 2);  // stress_{\phi \phi}
}
