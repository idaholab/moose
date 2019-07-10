#include "HeatFlux3EqnBC.h"
#include "HeatFluxFromHeatStructureBaseUserObject.h"
#include "THMIndices3Eqn.h"
#include "Assembly.h"

registerMooseObject("THMApp", HeatFlux3EqnBC);

template <>
InputParameters
validParams<HeatFlux3EqnBC>()
{
  InputParameters params = validParams<HeatFluxBaseBC>();
  params.addRequiredCoupledVar("rhoA", "rho*A of the flow channel");
  params.addRequiredCoupledVar("rhouA", "rhou*A of the flow channel");
  params.addRequiredCoupledVar("rhoEA", "rhoE*A of the flow channel");
  params.addRequiredCoupledVar("T_wall", "Wall temperature of the flow channel");
  return params;
}

HeatFlux3EqnBC::HeatFlux3EqnBC(const InputParameters & parameters)
  : HeatFluxBaseBC(parameters),
    _rhoA_jvar(coupled("rhoA")),
    _rhouA_jvar(coupled("rhouA")),
    _rhoEA_jvar(coupled("rhoEA")),
    _T_wall_jvar(coupled("T_wall")),
    _jvar_map(getVariableIndexMapping())
{
}

std::vector<unsigned int>
HeatFlux3EqnBC::getOffDiagVariableNumbers()
{
  return {_rhoA_jvar, _rhouA_jvar, _rhoEA_jvar};
}

Real
HeatFlux3EqnBC::computeQpResidual()
{
  const std::vector<Real> & q_wall = _q_uo.getHeatFlux(_current_elem->id());
  return -_hs_scale * q_wall[_qp] * _test[_i][_qp];
}

Real
HeatFlux3EqnBC::computeQpJacobian()
{
  const std::vector<DenseVector<Real>> & dq_wall = _q_uo.getHeatFluxJacobian(_current_elem->id());
  return -_hs_scale * dq_wall[_qp](_jvar_map.at(_var.number())) * _phi[_j][_qp] * _test[_i][_qp];
}

Real
HeatFlux3EqnBC::computeQpOffDiagJacobianNeighbor(unsigned int jvar)
{
  std::map<unsigned int, unsigned int>::const_iterator it;
  if ((it = _jvar_map.find(jvar)) != _jvar_map.end())
  {
    const std::vector<DenseVector<Real>> & dq_wall = _q_uo.getHeatFluxJacobian(_current_elem->id());
    return -_hs_scale * dq_wall[_qp](it->second) * _phi_neighbor[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0.;
}

std::map<unsigned int, unsigned int>
HeatFlux3EqnBC::getVariableIndexMapping() const
{
  std::map<unsigned int, unsigned int> jvar_map;
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhoA_jvar, THM3Eqn::EQ_MASS));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhouA_jvar, THM3Eqn::EQ_MOMENTUM));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhoEA_jvar, THM3Eqn::EQ_ENERGY));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_T_wall_jvar, THM3Eqn::EQ_ENERGY + 1));
  return jvar_map;
}
