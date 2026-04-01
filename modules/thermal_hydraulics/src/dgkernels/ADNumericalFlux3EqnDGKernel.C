//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNumericalFlux3EqnDGKernel.h"
#include "ADNumericalFlux3EqnBase.h"
#include "MooseVariable.h"
#include "THMIndicesVACE.h"

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
  params.addCoupledVar("passives_times_area", "Passive transport solution variables");

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
    _passives_times_area1(getADMaterialProperty<std::vector<Real>>("passives_times_area")),
    _p1(getADMaterialProperty<Real>("p")),
    _rhoA2(getNeighborADMaterialProperty<Real>("rhoA")),
    _rhouA2(getNeighborADMaterialProperty<Real>("rhouA")),
    _rhoEA2(getNeighborADMaterialProperty<Real>("rhoEA")),
    _passives_times_area2(getNeighborADMaterialProperty<std::vector<Real>>("passives_times_area")),
    _p2(getNeighborADMaterialProperty<Real>("p")),
    _numerical_flux(getUserObject<ADNumericalFlux3EqnBase>("numerical_flux")),
    _rhoA_var(coupled("rhoA")),
    _rhouA_var(coupled("rhouA")),
    _rhoEA_var(coupled("rhoEA")),
    _n_passives(isParamValid("passives_times_area") ? coupledComponents("passives_times_area") : 0),
    _jmap(getIndexMapping()),
    _equation_index(_jmap.at(_var.number()))
{
}

ADReal
ADNumericalFlux3EqnDGKernel::computeQpResidual(Moose::DGResidualType type)
{
  // left reconstructed solution
  std::vector<ADReal> U1(THMVACE1D::N_FLUX_INPUTS + _n_passives, 0.0);
  U1[THMVACE1D::RHOA] = _rhoA1[_qp];
  U1[THMVACE1D::RHOUA] = _rhouA1[_qp];
  U1[THMVACE1D::RHOEA] = _rhoEA1[_qp];
  U1[THMVACE1D::AREA] = _A_elem[_qp];
  for (const auto i : make_range(_n_passives))
    U1[THMVACE1D::N_FLUX_INPUTS + i] = _passives_times_area1[_qp][i];

  // right reconstructed solution
  std::vector<ADReal> U2(THMVACE1D::N_FLUX_INPUTS + _n_passives, 0.0);
  U2[THMVACE1D::RHOA] = _rhoA2[_qp];
  U2[THMVACE1D::RHOUA] = _rhouA2[_qp];
  U2[THMVACE1D::RHOEA] = _rhoEA2[_qp];
  U2[THMVACE1D::AREA] = _A_neig[_qp];
  for (const auto i : make_range(_n_passives))
    U2[THMVACE1D::N_FLUX_INPUTS + i] = _passives_times_area2[_qp][i];

  const Real nLR_dot_d = _current_side * 2 - 1.0;

  const std::vector<ADReal> & flux_elem =
      _numerical_flux.getFlux(_current_side, _current_elem->id(), true, U1, U2, nLR_dot_d);
  const std::vector<ADReal> & flux_neig =
      _numerical_flux.getFlux(_current_side, _current_elem->id(), false, U1, U2, nLR_dot_d);

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
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoA_var, THMVACE1D::MASS));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhouA_var, THMVACE1D::MOMENTUM));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoEA_var, THMVACE1D::ENERGY));
  for (const auto i : make_range(_n_passives))
    jmap.insert(std::pair<unsigned int, unsigned int>(coupled("passives_times_area", i),
                                                      THMVACE1D::N_FLUX_OUTPUTS + i));

  return jmap;
}
