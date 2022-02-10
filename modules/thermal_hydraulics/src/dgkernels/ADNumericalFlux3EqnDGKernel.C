//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNumericalFlux3EqnDGKernel.h"
#include "ADNumericalFlux3EqnBase.h"
#include "MooseVariable.h"
#include "THMIndices3Eqn.h"

registerMooseObject("ThermalHydraulicsApp", ADNumericalFlux3EqnDGKernel);

InputParameters
ADNumericalFlux3EqnDGKernel::validParams()
{
  InputParameters params = ADDGKernel::validParams();

  params.addClassDescription(
      "Adds side fluxes for the 1-D, 1-phase, variable-area Euler equations");

  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");
  params.addRequiredCoupledVar("rhoA", "Conserved variable: rho*A");
  params.addRequiredCoupledVar("rhouA", "Conserved variable: rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Conserved variable: rho*E*A");

  params.addRequiredParam<UserObjectName>("numerical_flux", "Name of numerical flux user object");

  return params;
}

ADNumericalFlux3EqnDGKernel::ADNumericalFlux3EqnDGKernel(const InputParameters & parameters)
  : ADDGKernel(parameters),

    _A_elem(adCoupledValue("A_linear")),
    _A_neig(adCoupledNeighborValue("A_linear")),
    _rhoA1(getADMaterialProperty<Real>("rhoA")),
    _rhouA1(getADMaterialProperty<Real>("rhouA")),
    _rhoEA1(getADMaterialProperty<Real>("rhoEA")),
    _p1(getADMaterialProperty<Real>("p")),
    _rhoA2(getNeighborADMaterialProperty<Real>("rhoA")),
    _rhouA2(getNeighborADMaterialProperty<Real>("rhouA")),
    _rhoEA2(getNeighborADMaterialProperty<Real>("rhoEA")),
    _p2(getNeighborADMaterialProperty<Real>("p")),
    _numerical_flux(getUserObject<ADNumericalFlux3EqnBase>("numerical_flux")),
    _rhoA_var(coupled("rhoA")),
    _rhouA_var(coupled("rhouA")),
    _rhoEA_var(coupled("rhoEA")),
    _jmap(getIndexMapping()),
    _equation_index(_jmap.at(_var.number()))
{
}

ADReal
ADNumericalFlux3EqnDGKernel::computeQpResidual(Moose::DGResidualType type)
{
  // construct the left and right solution vectors from the reconstructed solution
  std::vector<ADReal> U1 = {_rhoA1[_qp], _rhouA1[_qp], _rhoEA1[_qp], _A_elem[_qp]};
  std::vector<ADReal> U2 = {_rhoA2[_qp], _rhouA2[_qp], _rhoEA2[_qp], _A_neig[_qp]};

  const std::vector<ADReal> & flux_elem =
      _numerical_flux.getFlux(_current_side, _current_elem->id(), true, U1, U2, _normals[_qp](0));
  const std::vector<ADReal> & flux_neig =
      _numerical_flux.getFlux(_current_side, _current_elem->id(), false, U1, U2, _normals[_qp](0));

  ADReal re = 0.0;
  switch (type)
  {
    case Moose::Element:
      re = flux_elem[_equation_index] * _test[_i][_qp];
      break;
    case Moose::Neighbor:
      re = -flux_neig[_equation_index] * _test_neighbor[_i][_qp];
      break;
  }
  return re;
}

std::map<unsigned int, unsigned int>
ADNumericalFlux3EqnDGKernel::getIndexMapping() const
{
  std::map<unsigned int, unsigned int> jmap;
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoA_var, THM3Eqn::EQ_MASS));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhouA_var, THM3Eqn::EQ_MOMENTUM));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoEA_var, THM3Eqn::EQ_ENERGY));

  return jmap;
}
