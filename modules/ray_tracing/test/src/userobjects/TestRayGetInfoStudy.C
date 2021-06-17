//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestRayGetInfoStudy.h"

registerMooseObject("RayTracingTestApp", TestRayGetInfoStudy);

InputParameters
TestRayGetInfoStudy::validParams()
{
  auto params = LotsOfRaysRayStudy::validParams();
  return params;
}

TestRayGetInfoStudy::TestRayGetInfoStudy(const InputParameters & parameters)
  : LotsOfRaysRayStudy(parameters)
{
  registerRayData("data");
  registerRayAuxData("aux_data");
}

void
TestRayGetInfoStudy::onCompleteRay(const std::shared_ptr<Ray> & ray)
{
  LotsOfRaysRayStudy::onCompleteRay(ray);
  libMesh::err << ray->getInfo() << std::endl;
}
