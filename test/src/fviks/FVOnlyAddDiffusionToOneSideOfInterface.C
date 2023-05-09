//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
}

void
FVOnlyAddDiffusionToOneSideOfInterface::computeResidual(const FaceInfo & fi)
{
  setupData(fi);

  const auto var_elem_num = _elem_is_one ? var1().number() : var2().number();
  const auto var_neigh_num = _elem_is_one ? var2().number() : var1().number();

  const auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());

  if (_elem_is_one)
    processResidual(r, var_elem_num, false);
  else
    processResidual(-r, var_neigh_num, true);
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
  mooseAssert((elem_dof_indices.size() == 1) && (neigh_dof_indices.size() == 1),
              "We're currently built to use CONSTANT MONOMIALS");

  const auto r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

  if (_elem_is_one)
    _assembly.processResidualAndJacobian(r, elem_dof_indices[0], _vector_tags, _matrix_tags);
  else
    _assembly.processResidualAndJacobian(-r, neigh_dof_indices[0], _vector_tags, _matrix_tags);
}

ADReal
FVOnlyAddDiffusionToOneSideOfInterface::computeQpResidual()
{
  return _normal * -_coeff2(singleSidedFaceArg(var2()), Moose::currentState()) *
         var2().adGradSln(*_face_info, Moose::currentState());
}
