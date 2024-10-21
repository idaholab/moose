//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADBoundaryFlux3EqnBC.h"
#include "MooseVariable.h"
#include "THMIndicesVACE.h"

registerMooseObject("ThermalHydraulicsApp", ADBoundaryFlux3EqnBC);

InputParameters
ADBoundaryFlux3EqnBC::validParams()
{
  InputParameters params = ADOneDIntegratedBC::validParams();

  params.addClassDescription(
      "Boundary conditions for the 1-D, 1-phase, variable-area Euler equations");

  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");
  params.addRequiredCoupledVar("rhoA", "Conserved variable: rho*A");
  params.addRequiredCoupledVar("rhouA", "Conserved variable: rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Conserved variable: rho*E*A");

  params.addRequiredParam<UserObjectName>("boundary_flux", "Name of boundary flux user object");

  return params;
}

ADBoundaryFlux3EqnBC::ADBoundaryFlux3EqnBC(const InputParameters & parameters)
  : ADOneDIntegratedBC(parameters),

    _A_linear(adCoupledValue("A_linear")),

    _rhoA(getADMaterialProperty<Real>("rhoA")),
    _rhouA(getADMaterialProperty<Real>("rhouA")),
    _rhoEA(getADMaterialProperty<Real>("rhoEA")),

    _rhoA_var(coupled("rhoA")),
    _rhouA_var(coupled("rhouA")),
    _rhoEA_var(coupled("rhoEA")),

    _jmap(getIndexMapping()),
    _equation_index(_jmap.at(_var.number())),

    _flux(getUserObject<ADBoundaryFluxBase>("boundary_flux"))
{
}

ADReal
ADBoundaryFlux3EqnBC::computeQpResidual()
{
  const std::vector<ADReal> U = {_rhoA[_qp], _rhouA[_qp], _rhoEA[_qp], _A_linear[_qp]};
  const auto & flux =
      _flux.getFlux(_current_side, _current_elem->id(), U, MetaPhysicL::raw_value(_normals[_qp]));

  return flux[_equation_index] * _normal * _test[_i][_qp];
}

std::map<unsigned int, unsigned int>
ADBoundaryFlux3EqnBC::getIndexMapping() const
{
  std::map<unsigned int, unsigned int> jmap;
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoA_var, THMVACE1D::MASS));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhouA_var, THMVACE1D::MOMENTUM));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoEA_var, THMVACE1D::ENERGY));

  return jmap;
}
