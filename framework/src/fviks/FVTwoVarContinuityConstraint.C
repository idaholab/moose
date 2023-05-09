//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVTwoVarContinuityConstraint.h"

registerMooseObject("MooseApp", FVTwoVarContinuityConstraint);

InputParameters
FVTwoVarContinuityConstraint::validParams()
{
  InputParameters params = FVScalarLagrangeMultiplierInterface::validParams();
  params.addClassDescription(
      "Forces two variables to be equal on an interface for the finite volume method.");

  return params;
}

FVTwoVarContinuityConstraint::FVTwoVarContinuityConstraint(const InputParameters & params)
  : FVScalarLagrangeMultiplierInterface(params)
{
  if (&var1() == &var2())
    paramError("variable1",
               "FVTwoVarContinuityConstraint may not be applied on a single variable.");
}

ADReal
FVTwoVarContinuityConstraint::computeQpResidual()
{
  return var1().getBoundaryFaceValue(*_face_info, determineState()) -
         var2().getBoundaryFaceValue(*_face_info, determineState());
}
