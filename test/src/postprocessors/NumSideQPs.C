//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumSideQPs.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseTestApp", NumSideQPs);

InputParameters
NumSideQPs::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  return params;
}

NumSideQPs::NumSideQPs(const InputParameters & parameters) : SideIntegralPostprocessor(parameters)
{
}

NumSideQPs::~NumSideQPs() {}

Real
NumSideQPs::computeIntegral()
{
  return _qrule->n_points();
}

Real
NumSideQPs::computeQpIntegral()
{
  mooseError("Unimplemented method");
}
