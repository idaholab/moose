//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumElemQPs.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseTestApp", NumElemQPs);

InputParameters
NumElemQPs::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  return params;
}

NumElemQPs::NumElemQPs(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters)
{
}

NumElemQPs::~NumElemQPs() {}

Real
NumElemQPs::computeIntegral()
{
  return _qrule->n_points();
}

Real
NumElemQPs::computeQpIntegral()
{
  mooseError("Unimplemented method");
}
