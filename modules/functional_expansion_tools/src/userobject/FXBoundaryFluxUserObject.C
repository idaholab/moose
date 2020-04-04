//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FXBoundaryFluxUserObject.h"

registerMooseObject("FunctionalExpansionToolsApp", FXBoundaryFluxUserObject);

InputParameters
FXBoundaryFluxUserObject::validParams()
{
  InputParameters params = FXBoundaryBaseUserObject::validParams();

  params.addClassDescription("Generates an Functional Expansion representation for a boundary flux "
                             "condition using a 'FunctionSeries'-type Function");

  params.addRequiredParam<std::string>("diffusivity",
                                       "The name of the material diffusivity "
                                       "property, or raw value, that will be used "
                                       "in the flux computation.");

  return params;
}

FXBoundaryFluxUserObject::FXBoundaryFluxUserObject(const InputParameters & parameters)
  : FXBoundaryBaseUserObject(parameters),
    _diffusivity_name(parameters.get<std::string>("diffusivity")),
    _diffusivity(getMaterialProperty<Real>(_diffusivity_name))
{
}

Real
FXBoundaryFluxUserObject::computeQpIntegral()
{
  return -_diffusivity[_qp] * _grad_u[_qp] * _normals[_qp];
}
