//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayTracingAngularQuadratureErrorTest.h"

#include "RayTracingAngularQuadrature.h"
registerMooseObject("RayTracingTestApp", RayTracingAngularQuadratureErrorTest);

InputParameters
RayTracingAngularQuadratureErrorTest::validParams()
{
  auto params = GeneralUserObject::validParams();

  params.addParam<bool>("non_positive_polar_order", false, "Tests non-positive polar_order");
  params.addParam<bool>(
      "non_positive_azimuthal_order", false, "Tests non-positive azimuthal_order");
  params.addParam<bool>("mu_min_larger", false, "Tests mu_min being larger than mu_max");
  params.addParam<bool>("mu_min_too_small", false, "Tests mu_min < -1");
  params.addParam<bool>("mu_max_too_big", false, "Tests mu_max > 1");
  params.addParam<bool>("dim1", false, "Tests dimension 1");
  params.addParam<bool>(
      "non_positive_gauss_legendre_order", false, "Tests non-positive order in gauss-legendre quadrature");
  params.addParam<bool>("check_direction", false, "Checks for a non-valid direction index");
  params.addParam<bool>("orthonormal_vector_zero",
                        false,
                        "Tests getting an orthonormal vector to a vector that has a zero norm");

  return params;
}

RayTracingAngularQuadratureErrorTest::RayTracingAngularQuadratureErrorTest(
    const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
  if (getParam<bool>("non_positive_polar_order"))
    RayTracingAngularQuadrature(2, 0, 1, -1, 1);
  if (getParam<bool>("non_positive_azimuthal_order"))
    RayTracingAngularQuadrature(2, 1, 0, -1, 1);
  if (getParam<bool>("mu_min_larger"))
    RayTracingAngularQuadrature(2, 1, 1, 1, 0);
  if (getParam<bool>("mu_min_too_small"))
    RayTracingAngularQuadrature(2, 1, 1, -2, 0);
  if (getParam<bool>("mu_max_too_big"))
    RayTracingAngularQuadrature(2, 1, 1, -1, 2);
  if (getParam<bool>("dim1"))
    RayTracingAngularQuadrature(1, 1, 1, -1, 1);
  if (getParam<bool>("non_positive_gauss_legendre_order"))
  {
    std::vector<Real> x, y;
    RayTracingAngularQuadrature(2, 1, 1, -1, 1).gaussLegendre(0, x, y);
  }
  if (getParam<bool>("check_direction"))
    RayTracingAngularQuadrature(2, 1, 1, -1, 1).checkDirection(1337);
  if (getParam<bool>("orthonormal_vector_zero"))
    RayTracingAngularQuadrature::orthonormalVector(Point(0, 0, 0));
}
