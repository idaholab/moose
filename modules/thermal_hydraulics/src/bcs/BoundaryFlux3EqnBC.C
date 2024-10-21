//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryFlux3EqnBC.h"
#include "MooseVariable.h"
#include "THMIndicesVACE.h"

registerMooseObject("ThermalHydraulicsApp", BoundaryFlux3EqnBC);

InputParameters
BoundaryFlux3EqnBC::validParams()
{
  InputParameters params = OneDIntegratedBC::validParams();

  params.addClassDescription(
      "Boundary conditions for the 1-D, 1-phase, variable-area Euler equations");

  params.addRequiredCoupledVar("A_elem", "Cross-sectional area, elemental");
  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");
  params.addRequiredCoupledVar("rhoA", "Conserved variable: rho*A");
  params.addRequiredCoupledVar("rhouA", "Conserved variable: rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Conserved variable: rho*E*A");

  params.addRequiredParam<UserObjectName>("boundary_flux", "Name of boundary flux user object");

  return params;
}

BoundaryFlux3EqnBC::BoundaryFlux3EqnBC(const InputParameters & parameters)
  : OneDIntegratedBC(parameters),

    _A_elem(coupledValue("A_elem")),
    _A_linear(coupledValue("A_linear")),

    _rhoA(coupledValue("rhoA")),
    _rhouA(coupledValue("rhouA")),
    _rhoEA(coupledValue("rhoEA")),

    _rhoA_var(coupled("rhoA")),
    _rhouA_var(coupled("rhouA")),
    _rhoEA_var(coupled("rhoEA")),

    _jmap(getIndexMapping()),
    _equation_index(_jmap.at(_var.number())),

    _flux(getUserObject<BoundaryFluxBase>("boundary_flux"))
{
}

Real
BoundaryFlux3EqnBC::computeQpResidual()
{
  const std::vector<Real> U = {_rhoA[_qp], _rhouA[_qp], _rhoEA[_qp], _A_elem[_qp]};
  const auto & flux = _flux.getFlux(_current_side, _current_elem->id(), U, _normals[_qp]);

  // Note that the ratio A_linear / A_elem is necessary because A_elem is passed
  // to the flux function, but A_linear is to be used on the boundary.
  return flux[_equation_index] * _A_linear[_qp] / _A_elem[_qp] * _normal * _test[_i][_qp];
}

Real
BoundaryFlux3EqnBC::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
BoundaryFlux3EqnBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  const std::vector<Real> U = {_rhoA[_qp], _rhouA[_qp], _rhoEA[_qp], _A_elem[_qp]};
  const auto & J = _flux.getJacobian(_current_side, _current_elem->id(), U, {_normal, 0, 0});

  return J(_equation_index, _jmap.at(jvar)) * _A_linear[_qp] / _A_elem[_qp] * _normal *
         _phi[_j][_qp] * _test[_i][_qp];
}

std::map<unsigned int, unsigned int>
BoundaryFlux3EqnBC::getIndexMapping() const
{
  std::map<unsigned int, unsigned int> jmap;
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoA_var, THMVACE1D::MASS));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhouA_var, THMVACE1D::MOMENTUM));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoEA_var, THMVACE1D::ENERGY));

  return jmap;
}
