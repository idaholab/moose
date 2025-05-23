//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DarcyPressure.h"

registerMooseObject("BabblerApp", DarcyPressure);

InputParameters
DarcyPressure::validParams()
{
  InputParameters params = ADKernelGrad::validParams();
  params.addClassDescription("Compute the diffusion term for Darcy pressure ($p$) equation: "
                             "$-\\nabla \\cdot \\frac{\\mathbf{K}}{\\mu} \\nabla p = 0$");

  // Add a required parameter. If this isn't provided in the input file MOOSE will error.
  params.addRequiredParam<Real>("permeability", "The isotropic permeability ($K$) of the medium.");

  // Add an optional parameter and set its default value.
  params.addParam<Real>(
      "viscosity",
      7.98e-04,
      "The dynamic viscosity ($\\mu$) of the fluid, the default value is that of water at 30 "
      "degrees Celcius (7.98e-04 Pa-s).");

  return params;
}

DarcyPressure::DarcyPressure(const InputParameters & parameters)
  : ADKernelGrad(parameters),

    // Get the parameters from the input file
    _permeability(getParam<Real>("permeability")),
    _viscosity(getParam<Real>("viscosity"))
{
}

ADRealVectorValue
DarcyPressure::precomputeQpResidual()
{
  return (_permeability / _viscosity) * _grad_u[_qp];
}
