//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CosTheta.h"

registerMooseObject("ElectromagneticsTestApp", CosTheta);

InputParameters
CosTheta::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription("Function for cosine(theta) (where theta is in degrees) for use in "
                             "the slab reflection benchmark.");
  params.addRequiredParam<Real>("theta", "Angle (in degrees).");
  return params;
}

CosTheta::CosTheta(const InputParameters & parameters)
  : Function(parameters),

    _theta(getParam<Real>("theta"))
{
}

Real
CosTheta::value(Real /*t*/, const Point & /*p*/) const
{
  return std::cos(_theta * libMesh::pi / 180.);
}
