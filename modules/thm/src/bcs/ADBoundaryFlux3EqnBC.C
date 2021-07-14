#include "ADBoundaryFlux3EqnBC.h"
#include "MooseVariable.h"
#include "THMIndices3Eqn.h"

registerMooseObject("THMApp", ADBoundaryFlux3EqnBC);

InputParameters
ADBoundaryFlux3EqnBC::validParams()
{
  InputParameters params = ADOneDIntegratedBC::validParams();

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

ADBoundaryFlux3EqnBC::ADBoundaryFlux3EqnBC(const InputParameters & parameters)
  : ADOneDIntegratedBC(parameters),

    _A_elem(adCoupledValue("A_elem")),
    _A_linear(adCoupledValue("A_linear")),

    _rhoA(adCoupledValue("rhoA")),
    _rhouA(adCoupledValue("rhouA")),
    _rhoEA(adCoupledValue("rhoEA")),

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
  const std::vector<ADReal> U = {_rhoA[_qp], _rhouA[_qp], _rhoEA[_qp], _A_elem[_qp]};
  const auto & flux = _flux.getFlux(_current_side, _current_elem->id(), U, {_normal, 0, 0});

  // Note that the ratio A_linear / A_elem is necessary because A_elem is passed
  // to the flux function, but A_linear is to be used on the boundary.
  return flux[_equation_index] * _A_linear[_qp] / _A_elem[_qp] * _normal * _test[_i][_qp];
}

std::map<unsigned int, unsigned int>
ADBoundaryFlux3EqnBC::getIndexMapping() const
{
  std::map<unsigned int, unsigned int> jmap;
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoA_var, THM3Eqn::EQ_MASS));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhouA_var, THM3Eqn::EQ_MOMENTUM));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoEA_var, THM3Eqn::EQ_ENERGY));

  return jmap;
}
