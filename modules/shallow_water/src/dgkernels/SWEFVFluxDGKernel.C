//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWEFVFluxDGKernel.h"
#include "MooseVariable.h"

registerMooseObject("ShallowWaterApp", SWEFVFluxDGKernel);

InputParameters
SWEFVFluxDGKernel::validParams()
{
  InputParameters params = DGKernel::validParams();
  params.addClassDescription("DG side flux assembly for 2D shallow-water equations h, hu, hv.");
  params.addRequiredCoupledVar("h", "Conserved variable: h");
  params.addRequiredCoupledVar("hu", "Conserved variable: h*u");
  params.addRequiredCoupledVar("hv", "Conserved variable: h*v");
  params.addRequiredParam<UserObjectName>("numerical_flux", "Name of numerical flux user object");
  params.addRequiredCoupledVar(
      "b_var", "Cell-constant bathymetry variable (MONOMIAL/CONSTANT) to use at faces");
  return params;
}

SWEFVFluxDGKernel::SWEFVFluxDGKernel(const InputParameters & parameters)
  : DGKernel(parameters),
    _h1(getMaterialProperty<Real>("h")),
    _hu1(getMaterialProperty<Real>("hu")),
    _hv1(getMaterialProperty<Real>("hv")),
    _h2(getNeighborMaterialProperty<Real>("h")),
    _hu2(getNeighborMaterialProperty<Real>("hu")),
    _hv2(getNeighborMaterialProperty<Real>("hv")),
    _numerical_flux(getUserObject<InternalSideFluxBase>("numerical_flux")),
    _h_var(coupled("h")),
    _hu_var(coupled("hu")),
    _hv_var(coupled("hv")),
    _jmap(getIndexMapping()),
    _equation_index(_jmap.at(_var.number())),
    _b1_var(coupledValue("b_var")),
    _b2_var(coupledNeighborValue("b_var"))
{
}

SWEFVFluxDGKernel::~SWEFVFluxDGKernel() {}

Real
SWEFVFluxDGKernel::computeQpResidual(Moose::DGResidualType type)
{
  const Real bL = _b1_var[_qp];
  const Real bR = _b2_var[_qp];
  std::vector<Real> U1 = {_h1[_qp], _hu1[_qp], _hv1[_qp], bL};
  std::vector<Real> U2 = {_h2[_qp], _hu2[_qp], _hv2[_qp], bR};

  const auto & flux = _numerical_flux.getFlux(
      _current_side, _current_elem->id(), _neighbor_elem->id(), U1, U2, _normals[_qp]);

  switch (type)
  {
    case Moose::Element:
      return flux[_equation_index] * _test[_i][_qp];
    case Moose::Neighbor:
      return -flux[_equation_index] * _test_neighbor[_i][_qp];
  }
  return 0.0;
}

Real
SWEFVFluxDGKernel::computeQpJacobian(Moose::DGJacobianType type)
{
  return computeQpOffDiagJacobian(type, _var.number());
}

Real
SWEFVFluxDGKernel::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  const Real bL = _b1_var[_qp];
  const Real bR = _b2_var[_qp];
  std::vector<Real> U1 = {_h1[_qp], _hu1[_qp], _hv1[_qp], bL};
  std::vector<Real> U2 = {_h2[_qp], _hu2[_qp], _hv2[_qp], bR};

  const auto & dF_dUL = _numerical_flux.getJacobian(Moose::Element,
                                                    _current_side,
                                                    _current_elem->id(),
                                                    _neighbor_elem->id(),
                                                    U1,
                                                    U2,
                                                    _normals[_qp]);
  const auto & dF_dUR = _numerical_flux.getJacobian(Moose::Neighbor,
                                                    _current_side,
                                                    _current_elem->id(),
                                                    _neighbor_elem->id(),
                                                    U1,
                                                    U2,
                                                    _normals[_qp]);

  switch (type)
  {
    case Moose::ElementElement:
      return dF_dUL(_equation_index, _jmap.at(jvar)) * _phi[_j][_qp] * _test[_i][_qp];
    case Moose::ElementNeighbor:
      return dF_dUR(_equation_index, _jmap.at(jvar)) * _phi_neighbor[_j][_qp] * _test[_i][_qp];
    case Moose::NeighborElement:
      return -dF_dUL(_equation_index, _jmap.at(jvar)) * _phi[_j][_qp] * _test_neighbor[_i][_qp];
    case Moose::NeighborNeighbor:
      return -dF_dUR(_equation_index, _jmap.at(jvar)) * _phi_neighbor[_j][_qp] *
             _test_neighbor[_i][_qp];
  }
  return 0.0;
}

std::map<unsigned int, unsigned int>
SWEFVFluxDGKernel::getIndexMapping() const
{
  std::map<unsigned int, unsigned int> jmap;
  jmap.insert(std::make_pair(_h_var, 0));
  jmap.insert(std::make_pair(_hu_var, 1));
  jmap.insert(std::make_pair(_hv_var, 2));
  return jmap;
}
