//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVOnlyAddDiffusionToOneSideOfInterface.h"

registerMooseObject("MooseTestApp", FVOnlyAddDiffusionToOneSideOfInterface);

InputParameters
FVOnlyAddDiffusionToOneSideOfInterface::validParams()
{
  InputParameters params = FVInterfaceKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("coeff2",
                                                "The diffusion coefficient on the 2nd subdomains");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

FVOnlyAddDiffusionToOneSideOfInterface::FVOnlyAddDiffusionToOneSideOfInterface(
    const InputParameters & params)
  : FVInterfaceKernel(params), _coeff2(getFunctor<ADReal>("coeff2"))
{
  if (var1().sys().number() != var2().sys().number())
    mooseError(this->type(), " does not support multiple nonlinear systems!");
}

void
FVOnlyAddDiffusionToOneSideOfInterface::computeResidual(const FaceInfo & fi)
{
  setupData(fi);

  const auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());
  addResidualToVariable1(r);
}

void
FVOnlyAddDiffusionToOneSideOfInterface::computeResidualAndJacobian(const FaceInfo & fi)
{
  computeJacobian(fi);
}

void
FVOnlyAddDiffusionToOneSideOfInterface::computeJacobian(const FaceInfo & fi)
{
  setupData(fi);

  const auto r = fi.faceArea() * fi.faceCoord() * computeQpResidual();
  addResidualAndJacobianToVariable1(r);
}

ADReal
FVOnlyAddDiffusionToOneSideOfInterface::computeQpResidual()
{
  return -_coeff2(faceArg2(), Moose::currentState()) * normal() *
         var2().adGradSln(*_face_info, Moose::currentState());
}
