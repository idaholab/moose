//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialIntegralRayKernel.h"

registerMooseObject("RayTracingApp", MaterialIntegralRayKernel);

InputParameters
MaterialIntegralRayKernel::validParams()
{
  InputParameters params = IntegralRayKernel::validParams();

  params.addClassDescription("Integrates a Material property along a Ray.");

  params.addRequiredParam<MaterialPropertyName>("mat_prop",
                                                "The name of the material property to integrate");

  return params;
}

MaterialIntegralRayKernel::MaterialIntegralRayKernel(const InputParameters & params)
  : IntegralRayKernel(params), _mat(getMaterialProperty<Real>("mat_prop"))
{
}

Real
MaterialIntegralRayKernel::computeQpIntegral()
{
  return _mat[_qp];
}
