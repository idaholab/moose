//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MFEMCoordinateTransformations.h"

registerMooseObject("MooseApp", MFEMCoordinateTransformations);

InputParameters
MFEMCoordinateTransformations::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription("Base class for coordinate transformations used in MFEM problems.");
  params.addRequiredParam<MooseEnum>(
      "coord_type", MooseEnum("RZ"), "Coordinate system type. Currently only RZ is supported.");
  params.addParam<Real>(
      "inv_r_eps", 1e-12, "Regularization parameter used in inv_r = 1/sqrt(r^2 + eps^2).");
  return params;
}

MFEMCoordinateTransformations::MFEMCoordinateTransformations(const InputParameters & parameters)
  : Function(parameters),
    _coord_type(getParam<MooseEnum>("coord_type")),
    _inv_r_eps(getParam<Real>("inv_r_eps"))
{
}