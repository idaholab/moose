//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AEFVKernel.h"

registerMooseObject("RdgApp", AEFVKernel);

InputParameters
AEFVKernel::validParams()
{
  InputParameters params = DGKernel::validParams();
  params.addClassDescription(
      "A dgkernel for the advection equation using a cell-centered finite volume method.");
  MooseEnum component("concentration");
  params.addParam<MooseEnum>("component", component, "Choose one of the equations");
  params.addRequiredCoupledVar("u", "Name of the variable to use");
  params.addRequiredParam<UserObjectName>("flux", "Name of the internal side flux object to use");
  return params;
}

AEFVKernel::AEFVKernel(const InputParameters & parameters)
  : DGKernel(parameters),
    _component(getParam<MooseEnum>("component")),
    _uc1(coupledValue("u")),
    _uc2(coupledNeighborValue("u")),
    _u1(getMaterialProperty<Real>("u")),
    _u2(getNeighborMaterialProperty<Real>("u")),
    _flux(getUserObject<InternalSideFluxBase>("flux"))
{
}

AEFVKernel::~AEFVKernel() {}

Real
AEFVKernel::computeQpResidual(Moose::DGResidualType type)
{
  // assemble the input vectors, which are
  //   the reconstructed linear monomial
  //   extrapolated at side center from the current and neighbor elements
  std::vector<Real> uvec1 = {_u1[_qp]};
  std::vector<Real> uvec2 = {_u2[_qp]};

  // calculate the flux
  const auto & flux = _flux.getFlux(
      _current_side, _current_elem->id(), _neighbor_elem->id(), uvec1, uvec2, _normals[_qp]);

  // distribute the contribution to the current and neighbor elements
  switch (type)
  {
    case Moose::Element:
      return flux[_component] * _test[_i][_qp];

    case Moose::Neighbor:
      return -flux[_component] * _test_neighbor[_i][_qp];
  }

  return 0.0;
}

Real
AEFVKernel::computeQpJacobian(Moose::DGJacobianType type)
{
  // assemble the input vectors, which are
  //   the constant monomial from the current and neighbor elements
  std::vector<Real> uvec1 = {_uc1[_qp]};
  std::vector<Real> uvec2 = {_uc2[_qp]};

  // calculate the Jacobian matrices
  const auto & fjac1 = _flux.getJacobian(Moose::Element,
                                         _current_side,
                                         _current_elem->id(),
                                         _neighbor_elem->id(),
                                         uvec1,
                                         uvec2,
                                         _normals[_qp]);

  const auto & fjac2 = _flux.getJacobian(Moose::Neighbor,
                                         _current_side,
                                         _current_elem->id(),
                                         _neighbor_elem->id(),
                                         uvec1,
                                         uvec2,
                                         _normals[_qp]);

  // distribute the contribution to the current and neighbor elements
  switch (type)
  {
    case Moose::ElementElement:
      return fjac1(_component, _component) * _phi[_j][_qp] * _test[_i][_qp];

    case Moose::ElementNeighbor:
      return fjac2(_component, _component) * _phi_neighbor[_j][_qp] * _test[_i][_qp];

    case Moose::NeighborElement:
      return -fjac1(_component, _component) * _phi[_j][_qp] * _test_neighbor[_i][_qp];

    case Moose::NeighborNeighbor:
      return -fjac2(_component, _component) * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
  }

  return 0.0;
}
