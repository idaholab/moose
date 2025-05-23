//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVOnlyAddDiffusionToOneSideOfInterface.h"
#include "MathFVUtils.h"
#include "Assembly.h"

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

  const auto var_elem_num = _elem_is_one ? var1().number() : var2().number();
  const auto var_neigh_num = _elem_is_one ? var2().number() : var1().number();

  const auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());

  if (_elem_is_one)
    addResidual(r, var_elem_num, false);
  else
    addResidual(-r, var_neigh_num, true);
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

  const auto & elem_dof_indices = _elem_is_one ? var1().dofIndices() : var2().dofIndices();
  const auto & neigh_dof_indices =
      _elem_is_one ? var2().dofIndicesNeighbor() : var1().dofIndicesNeighbor();
  const auto elem_scaling_factor = _elem_is_one ? var1().scalingFactor() : var2().scalingFactor();
  const auto neighbor_scaling_factor =
      _elem_is_one ? var2().scalingFactor() : var1().scalingFactor();
  mooseAssert((elem_dof_indices.size() == 1) && (neigh_dof_indices.size() == 1),
              "We're currently built to use CONSTANT MONOMIALS");

  const auto r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

  if (_elem_is_one)
    addResidualsAndJacobian(
        _assembly, std::array<ADReal, 1>{{r}}, elem_dof_indices, elem_scaling_factor);
  else
    addResidualsAndJacobian(
        _assembly, std::array<ADReal, 1>{{-r}}, neigh_dof_indices, neighbor_scaling_factor);
}

ADReal
FVOnlyAddDiffusionToOneSideOfInterface::computeQpResidual()
{
  return _normal * -_coeff2(singleSidedFaceArg(var2()), Moose::currentState()) *
         var2().adGradSln(*_face_info, Moose::currentState());
}
