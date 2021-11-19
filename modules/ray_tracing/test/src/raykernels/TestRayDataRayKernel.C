//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestRayDataRayKernel.h"

// Local includes
#include "TestRayDataStudy.h"

registerMooseObject("RayTracingTestApp", TestRayDataRayKernel);

InputParameters
TestRayDataRayKernel::validParams()
{
  auto params = GeneralRayKernel::validParams();
  return params;
}

TestRayDataRayKernel::TestRayDataRayKernel(const InputParameters & params)
  : GeneralRayKernel(params), _test_ray_data_study(getStudy<TestRayDataStudy>())
{
}

void
TestRayDataRayKernel::onSegment()
{
  const auto & data_indices = _test_ray_data_study.dataIndices();

  for (unsigned int i = 0; i < data_indices.size(); ++i)
    currentRay()->data(data_indices[i]) +=
        _test_ray_data_study.dataValueChange(i, _current_segment_length);
}
