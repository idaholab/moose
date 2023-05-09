//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVBoundaryIntegralValueConstraint.h"

registerMooseObject("MooseApp", FVBoundaryIntegralValueConstraint);

InputParameters
FVBoundaryIntegralValueConstraint::validParams()
{
  InputParameters params = FVBoundaryScalarLagrangeMultiplierConstraint::validParams();
  params.addClassDescription(
      "This class is used to enforce integral of phi = boundary area * phi_0 "
      "with a Lagrange multiplier approach.");
  return params;
}

FVBoundaryIntegralValueConstraint::FVBoundaryIntegralValueConstraint(
    const InputParameters & parameters)
  : FVBoundaryScalarLagrangeMultiplierConstraint(parameters)
{
}

ADReal
FVBoundaryIntegralValueConstraint::computeQpResidual()
{
  return _var(makeFace(*_face_info, Moose::FV::LimiterType::CentralDifference, true),
              determineState()) -
         _phi0;
}
