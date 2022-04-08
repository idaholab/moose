//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IntegralRayKernel.h"

// Local includes
#include "RayTracingStudy.h"

InputParameters
IntegralRayKernel::validParams()
{
  auto params = IntegralRayKernelBase::validParams();
  params.addParam<bool>(
      "average",
      false,
      "Whether or not to compute the average value (divides by the segment length)");
  return params;
}

IntegralRayKernel::IntegralRayKernel(const InputParameters & params)
  : IntegralRayKernelBase(params),
    _integral_data_index(_study.registerRayData(integralRayDataName())),
    _average(getParam<bool>("average"))
{
}

void
IntegralRayKernel::onSegment()
{
  // Note that here we do not multiply by _coord[_qp]!
  //
  // The integral done here is the integral of a field variable/material/etc, and not
  // an integration that contributes to the residual/Jacobian. Hence: it is something like
  // a line integral. In RZ and RSPHERICAL, we want line integrals to still be line integrals.
  // Therefore, it does not make sense to multiply by the coordinate transformation.
  Real integral = 0;
  for (_qp = 0; _qp < _q_point.size(); ++_qp)
    integral += _JxW[_qp] * computeQpIntegral();

  // If we're computing the average, divide by the length
  if (_average)
    integral /= _current_segment_length;

  // Accumulate the integral into the Ray
  currentRay()->data(_integral_data_index) += integral;
}
