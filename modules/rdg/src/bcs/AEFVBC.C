//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AEFVBC.h"

registerMooseObject("RdgApp", AEFVBC);

InputParameters
AEFVBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription("A boundary condition kernel for the advection equation using a "
                             "cell-centered finite volume method.");
  MooseEnum component("concentration");
  params.addParam<MooseEnum>("component", component, "Choose one of the equations");
  params.addRequiredCoupledVar("u", "Name of the variable to use");
  params.addRequiredParam<UserObjectName>("flux", "Name of the boundary flux object to use");
  return params;
}

AEFVBC::AEFVBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _component(getParam<MooseEnum>("component")),
    _uc1(coupledValue("u")),
    _u1(getMaterialProperty<Real>("u")),
    _flux(getUserObject<BoundaryFluxBase>("flux"))
{
}

Real
AEFVBC::computeQpResidual()
{
  // assemble the input vectors, which are
  //   the reconstructed linear monomial
  //   extrapolated at side center from the current element
  std::vector<Real> uvec1 = {_u1[_qp]};

  // calculate the flux
  const auto & flux = _flux.getFlux(_current_side, _current_elem->id(), uvec1, _normals[_qp]);

  // distribute the contribution to the current element
  return flux[_component] * _test[_i][_qp];
}

Real
AEFVBC::computeQpJacobian()
{
  // assemble the input vectors, which are
  //   the constant monomial from the current element
  std::vector<Real> uvec1 = {_uc1[_qp]};

  // calculate the flux
  auto & fjac1 = _flux.getJacobian(_current_side, _current_elem->id(), uvec1, _normals[_qp]);

  // distribute the contribution to the current element
  return fjac1(_component, _component) * _phi[_j][_qp] * _test[_i][_qp];
}
