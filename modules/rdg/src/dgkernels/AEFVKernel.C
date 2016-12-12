/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AEFVKernel.h"

template<>
InputParameters validParams<AEFVKernel>()
{
  InputParameters params = validParams<DGKernel>();
  params.addClassDescription("A dgkernel for the advection equation using a cell-centered finite volume method.");
  MooseEnum component("concentration");
  params.addParam<MooseEnum>("component", component, "Choose one of the equations");
  params.addRequiredCoupledVar("u", "Name of the variable to use");
  params.addRequiredParam<UserObjectName>("flux", "Name of the internal side flux object to use");
  return params;
}

AEFVKernel::AEFVKernel(const InputParameters & parameters) :
    DGKernel(parameters),
    _component(getParam<MooseEnum>("component")),
    _uc1(coupledValue("u")),
    _uc2(coupledNeighborValue("u")),
    _u1(getMaterialProperty<Real>("u")),
    _u2(getNeighborMaterialProperty<Real>("u")),
    _flux(getUserObject<InternalSideFluxBase>("flux"))
{
}

AEFVKernel::~AEFVKernel()
{
}

Real
AEFVKernel::computeQpResidual(Moose::DGResidualType type)
{
  // assembly the input vectors, which are
  //   the reconstructed linear monomial
  //   extrapolated at side center from the current and neighbor elements
  //   and the unit vector normal to the face

  std::vector<Real> uvec1 = {_u1[_qp]};
  std::vector<Real> uvec2 = {_u2[_qp]};
  std::vector<Real> dwave = {_normals[_qp](0), _normals[_qp](1), _normals[_qp](2)};

  // calculate the flux

  const std::vector<Real> & flux =
    _flux.getFlux(_current_side, _current_elem->id(), _neighbor_elem->id(),
                  uvec1, uvec2, dwave, _tid);

  // distribute the contribution to the current and neighbor elements

  Real re = 0.;
  switch (type)
  {
    case Moose::Element:
      re =  flux[_component] * _test[_i][_qp];
      break;
    case Moose::Neighbor:
      re = -flux[_component] * _test_neighbor[_i][_qp];
      break;
  }
  return re;
}

Real
AEFVKernel::computeQpJacobian(Moose::DGJacobianType type)
{
  // assembly the input vectors, which are
  //   the constant monomial from the current and neighbor elements
  //   and the unit vector normal to the face

  std::vector<Real> uvec1 = { _uc1[_qp]};
  std::vector<Real> uvec2 = { _uc2[_qp]};
  std::vector<Real> dwave = {_normals[_qp](0), _normals[_qp](1), _normals[_qp](2)};

  // calculate the Jacobian matrix

  const DenseMatrix<Real> & fjac1 =
    _flux.getJacobian(Moose::Element, _current_side, _current_elem->id(), _neighbor_elem->id(),
                      uvec1, uvec2, dwave, _tid);

  const DenseMatrix<Real> & fjac2 =
    _flux.getJacobian(Moose::Neighbor, _current_side, _current_elem->id(), _neighbor_elem->id(),
                      uvec1, uvec2, dwave, _tid);

  // distribute the contribution to the current and neighbor elements

  Real re = 0.;
  switch (type)
  {
    case Moose::ElementElement:
      re =  fjac1(_component, _component) * _phi[_j][_qp] * _test[_i][_qp];
      break;
    case Moose::ElementNeighbor:
      re =  fjac2(_component, _component) * _phi_neighbor[_j][_qp] * _test[_i][_qp];
      break;
    case Moose::NeighborElement:
      re = -fjac1(_component, _component) * _phi[_j][_qp] * _test_neighbor[_i][_qp];
      break;
    case Moose::NeighborNeighbor:
      re = -fjac2(_component, _component) * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
      break;
  }
  return re;
}
