/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AEFVBC.h"

template<>
InputParameters validParams<AEFVBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addClassDescription("A boundary condition kernel for the advection equation using a cell-centered finite volume method.");
  MooseEnum component("concentration");
  params.addParam<MooseEnum>("component", component, "Choose one of the equations");
  params.addRequiredCoupledVar("u", "Name of the variable to use");
  params.addRequiredParam<UserObjectName>("flux", "Name of the boundary flux object to use");
  return params;
}

AEFVBC::AEFVBC(const InputParameters & parameters) :
    IntegratedBC(parameters),
    _component(getParam<MooseEnum>("component")),
    _uc1(coupledValue("u")),
    _u1(getMaterialProperty<Real>("u")),
    _flux(getUserObject<BoundaryFluxBase>("flux"))
{
}

Real
AEFVBC::computeQpResidual()
{
  // assembly the input vectors, which are
  //   the reconstructed linear monomial
  //   extrapolated at side center from the current element
  //   and the unit vector normal to the face

  std::vector<Real> uvec1 = {_u1[_qp]};
  std::vector<Real> dwave = {_normals[_qp](0), _normals[_qp](1), _normals[_qp](2)};

  // calculate the flux

  const std::vector<Real> & flux =
    _flux.getFlux(_current_side, _current_elem->id(), uvec1, dwave, _tid);

  // distribute the contribution to the current element

  return flux[_component] * _test[_i][_qp];
}

Real
AEFVBC::computeQpJacobian()
{
  // assembly the input vectors, which are
  //   the constant monomial from the current element
  //   and the unit vector normal to the face

  std::vector<Real> uvec1 = {_uc1[_qp]};
  std::vector<Real> dwave = {_normals[_qp](0), _normals[_qp](1), _normals[_qp](2)};

  // calculate the flux

  const DenseMatrix<Real> & fjac1 =
    _flux.getJacobian(_current_side, _current_elem->id(), uvec1, dwave, _tid);

  // distribute the contribution to the current element

  return fjac1(_component, _component) * _phi[_j][_qp] * _test[_i][_qp];
}
