//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryFlux1PhaseBaseBC.h"
#include "MooseVariable.h"

InputParameters
BoundaryFlux1PhaseBaseBC::validParams()
{
  InputParameters params = ADOneDIntegratedBC::validParams();

  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");
  params.addRequiredCoupledVar("rhoA", "Conserved variable: rho*A");
  params.addRequiredCoupledVar("rhouA", "Conserved variable: rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Conserved variable: rho*E*A");

  params.addRequiredParam<UserObjectName>("boundary_flux", "Name of boundary flux user object");

  return params;
}

BoundaryFlux1PhaseBaseBC::BoundaryFlux1PhaseBaseBC(const InputParameters & parameters)
  : ADOneDIntegratedBC(parameters),

    _A_linear(adCoupledValue("A_linear")),

    _rhoA(getADMaterialProperty<Real>("rhoA")),
    _rhouA(getADMaterialProperty<Real>("rhouA")),
    _rhoEA(getADMaterialProperty<Real>("rhoEA")),

    _rhoA_var(coupled("rhoA")),
    _rhouA_var(coupled("rhouA")),
    _rhoEA_var(coupled("rhoEA")),

    _flux(getUserObject<ADBoundaryFluxBase>("boundary_flux"))
{
}

void
BoundaryFlux1PhaseBaseBC::initialSetup()
{
  ADOneDIntegratedBC::initialSetup();

  const auto jmap = getIndexMapping();
  _equation_index = jmap.at(_var.number());
}

ADReal
BoundaryFlux1PhaseBaseBC::computeQpResidual()
{
  const auto & flux = _flux.getFlux(
      _current_side, _current_elem->id(), fluxInputVector(), MetaPhysicL::raw_value(_normals[_qp]));

  return flux[_equation_index] * _normal * _test[_i][_qp];
}
