//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMaterialPropertySource.h"

registerMooseObject("MooseTestApp", ADMaterialPropertySource);

InputParameters
ADMaterialPropertySource::validParams()
{
  InputParameters params = ADInterfaceKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("source",
                                                "The material property that is the flux source.");
  return params;
}

ADMaterialPropertySource::ADMaterialPropertySource(const InputParameters & parameters)
  : ADInterfaceKernel(parameters), _source(getADMaterialProperty<Real>("source"))
{
}

ADReal
ADMaterialPropertySource::computeQpResidual(Moose::DGResidualType type)
{
  ADReal r = 0;

  switch (type)
  {
    case Moose::Element:
      r = _test[_i][_qp] * _source[_qp];
      break;

    case Moose::Neighbor:
      r = -_test_neighbor[_i][_qp] * _source[_qp];
      break;
  }

  return r;
}
