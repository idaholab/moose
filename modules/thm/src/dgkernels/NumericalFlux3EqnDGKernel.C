#include "NumericalFlux3EqnDGKernel.h"
#include "MooseVariable.h"
#include "RELAP7Indices3Eqn.h"

registerMooseObject("RELAP7App", NumericalFlux3EqnDGKernel);

template <>
InputParameters
validParams<NumericalFlux3EqnDGKernel>()
{
  InputParameters params = validParams<DGKernel>();

  params.addClassDescription(
      "Adds side fluxes for the 1-D, 1-phase, variable-area Euler equations");

  params.addRequiredCoupledVar("A", "Cross-sectional area, elemental");
  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");
  params.addRequiredCoupledVar("rhoA", "Conserved variable: rho*A");
  params.addRequiredCoupledVar("rhouA", "Conserved variable: rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Conserved variable: rho*E*A");

  params.addRequiredParam<UserObjectName>("numerical_flux", "Name of numerical flux user object");

  return params;
}

NumericalFlux3EqnDGKernel::NumericalFlux3EqnDGKernel(const InputParameters & parameters)
  : DGKernel(parameters),

    _A_linear(coupledValue("A_linear")),
    _A1_avg(coupledValue("A")),
    _rhoA1_avg(coupledValue("rhoA")),
    _rhouA1_avg(coupledValue("rhouA")),
    _rhoEA1_avg(coupledValue("rhoEA")),
    _A2_avg(coupledNeighborValue("A")),
    _rhoA2_avg(coupledNeighborValue("rhoA")),
    _rhouA2_avg(coupledNeighborValue("rhouA")),
    _rhoEA2_avg(coupledNeighborValue("rhoEA")),
    _rhoA1(getMaterialProperty<Real>("rhoA")),
    _rhouA1(getMaterialProperty<Real>("rhouA")),
    _rhoEA1(getMaterialProperty<Real>("rhoEA")),
    _p1(getMaterialProperty<Real>("p")),
    _rhoA2(getNeighborMaterialProperty<Real>("rhoA")),
    _rhouA2(getNeighborMaterialProperty<Real>("rhouA")),
    _rhoEA2(getNeighborMaterialProperty<Real>("rhoEA")),
    _p2(getNeighborMaterialProperty<Real>("p")),
    _numerical_flux(getUserObject<RDGFluxBase>("numerical_flux")),
    _rhoA_var(coupled("rhoA")),
    _rhouA_var(coupled("rhouA")),
    _rhoEA_var(coupled("rhoEA")),
    _jmap(getIndexMapping()),
    _equation_index(_jmap.at(_var.number()))
{
}

Real
NumericalFlux3EqnDGKernel::computeQpResidual(Moose::DGResidualType type)
{
  // construct the left and right solution vectors from the reconstructed solution
  std::vector<Real> U1 = {_rhoA1[_qp], _rhouA1[_qp], _rhoEA1[_qp], _A_linear[_qp]};
  std::vector<Real> U2 = {_rhoA2[_qp], _rhouA2[_qp], _rhoEA2[_qp], _A_linear[_qp]};

  const std::vector<Real> & flux =
      _numerical_flux.getFlux(_current_side, _current_elem->id(), U1, U2, _normals[_qp]);

  Real re = 0.0;
  switch (type)
  {
    case Moose::Element:
      re = flux[_equation_index] * _test[_i][_qp];
      break;
    case Moose::Neighbor:
      re = -flux[_equation_index] * _test_neighbor[_i][_qp];
      break;
  }
  return re;
}

Real
NumericalFlux3EqnDGKernel::computeQpJacobian(Moose::DGJacobianType type)
{
  return computeQpOffDiagJacobian(type, _var.number());
}

Real
NumericalFlux3EqnDGKernel::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  // construct the left and right solution vectors from the cell-average solution
  std::vector<Real> U1 = {_rhoA1_avg[_qp], _rhouA1_avg[_qp], _rhoEA1_avg[_qp], _A1_avg[_qp]};
  std::vector<Real> U2 = {_rhoA2_avg[_qp], _rhouA2_avg[_qp], _rhoEA2_avg[_qp], _A2_avg[_qp]};

  const DenseMatrix<Real> & fjac1 =
      _numerical_flux.getJacobian(true, _current_side, _current_elem->id(), U1, U2, _normals[_qp]);

  const DenseMatrix<Real> & fjac2 =
      _numerical_flux.getJacobian(false, _current_side, _current_elem->id(), U1, U2, _normals[_qp]);

  Real re = 0.0;
  switch (type)
  {
    case Moose::ElementElement:
      re = fjac1(_equation_index, _jmap.at(jvar)) * _phi[_j][_qp] * _test[_i][_qp];
      break;
    case Moose::ElementNeighbor:
      re = fjac2(_equation_index, _jmap.at(jvar)) * _phi_neighbor[_j][_qp] * _test[_i][_qp];
      break;
    case Moose::NeighborElement:
      re = -fjac1(_equation_index, _jmap.at(jvar)) * _phi[_j][_qp] * _test_neighbor[_i][_qp];
      break;
    case Moose::NeighborNeighbor:
      re = -fjac2(_equation_index, _jmap.at(jvar)) * _phi_neighbor[_j][_qp] *
           _test_neighbor[_i][_qp];
      break;
  }
  return re;
}

std::map<unsigned int, unsigned int>
NumericalFlux3EqnDGKernel::getIndexMapping() const
{
  std::map<unsigned int, unsigned int> jmap;
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoA_var, RELAP73Eqn::EQ_MASS));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhouA_var, RELAP73Eqn::EQ_MOMENTUM));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoEA_var, RELAP73Eqn::EQ_ENERGY));

  return jmap;
}
