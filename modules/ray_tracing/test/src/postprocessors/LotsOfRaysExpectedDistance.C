//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LotsOfRaysExpectedDistance.h"

// Local includes
#include "LotsOfRaysRayStudy.h"

registerMooseObject("RayTracingTestApp", LotsOfRaysExpectedDistance);

InputParameters
LotsOfRaysExpectedDistance::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addRequiredParam<UserObjectName>(
      "lots_of_rays_study", "The LotsOfRaysRayStudy to get the expected distance from");

  return params;
}

LotsOfRaysExpectedDistance::LotsOfRaysExpectedDistance(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _study(getUserObject<LotsOfRaysRayStudy>("lots_of_rays_study"))
{
  if (!_study.hasExpectedDistance())
    mooseError(type(),
               " '",
               name(),
               ": The LotsOfRaysRayStudy '",
               _study.name(),
               "' does not have compute_expected_distance = true");
}

Real
LotsOfRaysExpectedDistance::getValue()
{
  return _study.expectedDistance();
}
