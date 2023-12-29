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
#include "Assembly.h"

registerMooseObject("TensorMechanicsApp", ADStressDivergenceRSphericalTensors);

InputParameters
ADStressDivergenceRSphericalTensors::validParams()
{
  InputParameters params = ADStressDivergenceTensors::validParams();
  params.addClassDescription(
      "Calculate stress divergence for a spherically symmetric 1D problem in polar coordinates.");
  params.set<unsigned int>("component") = 0;
  return params;
}

ADStressDivergenceRSphericalTensors::ADStressDivergenceRSphericalTensors(
    const InputParameters & parameters)
  : ADStressDivergenceTensors(parameters)
{
  if (_component != 0)
    mooseError("Invalid component for this 1D RSpherical problem.");
}

void
ADStressDivergenceRSphericalTensors::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_RSPHERICAL)
    mooseError("The coordinate system in the Problem block must be set to RSPHERICAL for 1D "
               "spherically symmetric geometries.");
}

ADReal
ADStressDivergenceRSphericalTensors::computeQpResidual()
{
  return _grad_test[_i][_qp](0) * _stress[_qp](0, 0) +                 // stress_{rr} part 1
         (_test[_i][_qp] / _ad_q_point[_qp](0)) * _stress[_qp](1, 1) + // stress_{\theta \theta}
         (_test[_i][_qp] / _ad_q_point[_qp](0)) * _stress[_qp](2, 2);  // stress_{\phi \phi}
}
