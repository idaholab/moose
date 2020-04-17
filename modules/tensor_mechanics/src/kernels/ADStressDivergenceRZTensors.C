//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADStressDivergenceRZTensors.h"
#include "Assembly.h"
#include "RankTwoTensor.h"
#include "ElasticityTensorTools.h"
#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", ADStressDivergenceRZTensors);

InputParameters
ADStressDivergenceRZTensors::validParams()
{
  InputParameters params = ADStressDivergenceTensors::validParams();
  params.addClassDescription(
      "Calculate stress divergence for an axisymmetric problem in cylindrical coordinates.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component",
      "component < 2",
      "An integer corresponding to the direction the variable this kernel acts in. (0 "
      "refers to the radial and 1 to the axial displacement.)");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

ADStressDivergenceRZTensors::ADStressDivergenceRZTensors(const InputParameters & parameters)
  : ADStressDivergenceTensors(parameters)
{
}

void
ADStressDivergenceRZTensors::initialSetup()
{
  if (getBlockCoordSystem() != Moose::COORD_RZ)
    mooseError("The coordinate system in the Problem block must be set to RZ for axisymmetric "
               "geometries.");
}

ADReal
ADStressDivergenceRZTensors::computeQpResidual()
{
  ADReal div = 0.0;
  if (_component == 0)
  {
    div = _grad_test[_i][_qp](0) * _stress[_qp](0, 0) +
          (_test[_i][_qp] / _ad_q_point[_qp](0)) * _stress[_qp](2, 2) +
          _grad_test[_i][_qp](1) * _stress[_qp](0, 1); // stress_{rz}

    // volumetric locking correction
    if (_volumetric_locking_correction)
      div += (_avg_grad_test[_i] - _grad_test[_i][_qp](0) - _test[_i][_qp] / _ad_q_point[_qp](0)) *
             (_stress[_qp].trace()) / 3.0;
  }
  else if (_component == 1)
  {
    div = _grad_test[_i][_qp](1) * _stress[_qp](1, 1) +
          _grad_test[_i][_qp](0) * _stress[_qp](1, 0); // stress_{zr}

    // volumetric locking correction
    if (_volumetric_locking_correction)
      div += (_avg_grad_test[_i] - _grad_test[_i][_qp](1)) * (_stress[_qp].trace()) / 3.0;
  }
  else
    mooseError("Invalid component for this AxisymmetricRZ problem.");

  return div;
}

void
ADStressDivergenceRZTensors::precalculateResidual()
{
  if (!_volumetric_locking_correction)
    return;

  ADReal ad_current_elem_volume = 0.0;
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
    ad_current_elem_volume += _ad_JxW[qp] * _ad_coord[qp];

  // calculate volume averaged value of shape function derivative
  _avg_grad_test.resize(_test.size());
  for (_i = 0; _i < _test.size(); ++_i)
  {
    _avg_grad_test[_i] = 0.0;
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      if (_component == 0)
        _avg_grad_test[_i] +=
            (_grad_test[_i][_qp](_component) + _test[_i][_qp] / _ad_q_point[_qp](0)) *
            _ad_JxW[_qp] * _ad_coord[_qp];
      else
        _avg_grad_test[_i] += _grad_test[_i][_qp](_component) * _ad_JxW[_qp] * _ad_coord[_qp];
    }
    _avg_grad_test[_i] /= ad_current_elem_volume;
  }
}
