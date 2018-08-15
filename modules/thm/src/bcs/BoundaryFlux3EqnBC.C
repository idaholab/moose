#include "BoundaryFlux3EqnBC.h"
#include "MooseVariable.h"

registerMooseObject("RELAP7App", BoundaryFlux3EqnBC);

template <>
InputParameters
validParams<BoundaryFlux3EqnBC>()
{
  InputParameters params = validParams<OneDIntegratedBC>();

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
    RDGIndices3Eqn(),

    _A_avg(coupledValue("A_elem")),
    _A_linear(coupledValue("A_linear")),
    _rhoA_avg(coupledValue("rhoA")),
    _rhouA_avg(coupledValue("rhouA")),
    _rhoEA_avg(coupledValue("rhoEA")),
    _rhoA(getMaterialProperty<Real>("rhoA")),
    _rhouA(getMaterialProperty<Real>("rhouA")),
    _rhoEA(getMaterialProperty<Real>("rhoEA")),
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
  // construct the interior solution vector from the reconstructed solution
  const std::vector<Real> U = {_rhoA[_qp], _rhouA[_qp], _rhoEA[_qp], _A_linear[_qp]};

  const auto & flux = _flux.getFlux(_current_side, _current_elem->id(), U, {_normal, 0, 0}, _tid);

  return flux[_equation_index] * _normal * _test[_i][_qp];
}

Real
BoundaryFlux3EqnBC::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
BoundaryFlux3EqnBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  // construct the interior solution vector from the cell-average solution
  const std::vector<Real> U = {_rhoA_avg[_qp], _rhouA_avg[_qp], _rhoEA_avg[_qp], _A_avg[_qp]};

  const auto & J = _flux.getJacobian(_current_side, _current_elem->id(), U, {_normal, 0, 0}, _tid);

  return J(_equation_index, _jmap.at(jvar)) * _normal * _phi[_j][_qp] * _test[_i][_qp];
}

std::map<unsigned int, unsigned int>
BoundaryFlux3EqnBC::getIndexMapping() const
{
  std::map<unsigned int, unsigned int> jmap;
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoA_var, EQ_MASS));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhouA_var, EQ_MOMENTUM));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoEA_var, EQ_ENERGY));

  return jmap;
}
