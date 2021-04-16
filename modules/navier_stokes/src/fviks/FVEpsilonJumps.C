//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVEpsilonJumps.h"
#include "FVUtils.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", FVEpsilonJumps);

InputParameters
FVEpsilonJumps::validParams()
{
  InputParameters params = FVInterfaceKernel::validParams();
  params.addClassDescription("Computes p * grad_eps when grad_eps is a delta function at a face.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

FVEpsilonJumps::FVEpsilonJumps(const InputParameters & params)
  : FVInterfaceKernel(params),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity)),
    _pressure_elem(getADMaterialProperty<Real>(NS::pressure)),
    _pressure_neighbor(getNeighborADMaterialProperty<Real>(NS::pressure)),
    _index(getParam<MooseEnum>("momentum_component"))
{
  if (&var1() != &var2())
    paramError("variable2",
               name(),
               " is only designed to work with the same variable on both sides of an interface.");
}

ADReal
FVEpsilonJumps::computeQpResidual()
{
  return -_normal(_index) * (_eps_neighbor[_qp] - _eps_elem[_qp]) / 2;
}

void
FVEpsilonJumps::computeResidual(const FaceInfo & fi)
{
  setupData(fi);

  const auto var_elem_num = elemIsOne() ? var1().number() : var2().number();
  const auto var_neigh_num = elemIsOne() ? var2().number() : var1().number();

  const auto r_elem = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual() *
                                             _pressure_elem[_qp]);

  processResidual(r_elem, var_elem_num, false);

  const auto r_neighbor = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() *
                                                 computeQpResidual() * _pressure_neighbor[_qp]);

  processResidual(r_neighbor, var_neigh_num, true);
}

void
FVEpsilonJumps::computeJacobian(const FaceInfo & fi)
{
  setupData(fi);

  const auto & elem_dof_indices = elemIsOne() ? var1().dofIndices() : var2().dofIndices();
  const auto & neigh_dof_indices =
      elemIsOne() ? var2().dofIndicesNeighbor() : var1().dofIndicesNeighbor();
  mooseAssert((elem_dof_indices.size() == 1) && (neigh_dof_indices.size() == 1),
              "We're currently built to use CONSTANT MONOMIALS");

  const auto r_elem = fi.faceArea() * fi.faceCoord() * computeQpResidual() * _pressure_elem[_qp];
  processDerivatives(r_elem, elem_dof_indices[0]);

  const auto r_neighbor =
      fi.faceArea() * fi.faceCoord() * computeQpResidual() * _pressure_neighbor[_qp];
  processDerivatives(r_neighbor, neigh_dof_indices[0]);
}
