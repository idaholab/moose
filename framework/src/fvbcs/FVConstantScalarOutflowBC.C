//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVConstantScalarOutflowBC.h"

registerMooseObject("MooseApp", FVConstantScalarOutflowBC);

InputParameters
FVConstantScalarOutflowBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription(
      "Constant velocity scalar advection boundary conditions for finite volume method.");
  params.addRequiredParam<RealVectorValue>("velocity", "Constant advection velocity");
  return params;
}

FVConstantScalarOutflowBC::FVConstantScalarOutflowBC(const InputParameters & parameters)
  : FVFluxBC(parameters), _velocity(getParam<RealVectorValue>("velocity"))
{
}

ADReal
FVConstantScalarOutflowBC::computeQpResidual()
{
  mooseAssert(_normal * _velocity >= 0,
              "This boundary condition is for outflow but the flow is in the opposite direction of "
              "the boundary normal");

  const auto boundary_face = singleSidedFaceArg();
  const auto state = determineState();

  // This will either be second or first order accurate depending on whether the user has asked
  // for a two term expansion in their input file
  return _normal * _velocity * _var(boundary_face, state);
}
