//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// This file is part of the MOOSE framework
// https://mooseframework.inl.gov
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoefVectorTimeDerivative.h"

/**
 * Register with the Electromagnetics module app.
 * This makes the kernel available in input files as
 * type = CoefVectorTimeDerivative
 */
registerMooseObject("ElectromagneticsApp", CoefVectorTimeDerivative);

InputParameters
CoefVectorTimeDerivative::validParams()
{
  InputParameters params = VectorTimeDerivative::validParams();
  params.addRequiredParam<Real>("Coefficient",
                                "Scalar coefficient multiplying the vector time derivative "
                                "(e.g. mu0 * sigma for the magnetoquasistatic EM equation)");
  return params;
}

CoefVectorTimeDerivative::CoefVectorTimeDerivative(const InputParameters & parameters)
  : // Must call the base class constructor first
    VectorTimeDerivative(parameters),
    _coef(getParam<Real>("Coefficient"))
{
}

Real
CoefVectorTimeDerivative::computeQpResidual()
{
  return _coef * VectorTimeDerivative::computeQpResidual();
}

Real
CoefVectorTimeDerivative::computeQpJacobian()
{
  return _coef * VectorTimeDerivative::computeQpJacobian();
}
