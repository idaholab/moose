#include "OneD3EqnEnergyHeatFlux.h"
#include "HeatFluxFromHeatStructureBaseUserObject.h"
#include "THMIndices3Eqn.h"

registerMooseObject("THMApp", OneD3EqnEnergyHeatFlux);

template <>
InputParameters
validParams<OneD3EqnEnergyHeatFlux>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<UserObjectName>(
      "q_uo", "The name of the user object that computed the heat flux");
  params.addRequiredCoupledVar("P_hf", "Heat flux perimeter");
  params.addRequiredCoupledVar("rhoA", "rho*A of the flow channel");
  params.addRequiredCoupledVar("rhouA", "rhou*A of the flow channel");
  params.addRequiredCoupledVar("rhoEA", "rhoE*A of the flow channel");
  return params;
}

OneD3EqnEnergyHeatFlux::OneD3EqnEnergyHeatFlux(const InputParameters & parameters)
  : Kernel(parameters),
    _q_uo(getUserObject<HeatFluxFromHeatStructureBaseUserObject>("q_uo")),
    _P_hf(coupledValue("P_hf")),
    _rhoA_jvar(coupled("rhoA")),
    _rhouA_jvar(coupled("rhouA")),
    _rhoEA_jvar(coupled("rhoEA")),
    _jvar_map(getVariableIndexMapping())
{
}

Real
OneD3EqnEnergyHeatFlux::computeQpResidual()
{
  const std::vector<Real> & q_wall = _q_uo.getHeatFlux(_current_elem->id());
  return -q_wall[_qp] * _test[_i][_qp];
}

Real
OneD3EqnEnergyHeatFlux::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
OneD3EqnEnergyHeatFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  auto it = _jvar_map.find(jvar);
  if (it != _jvar_map.end())
  {
    const std::vector<DenseVector<Real>> & dq_wall = _q_uo.getHeatFluxJacobian(_current_elem->id());
    return -dq_wall[_qp](it->second) * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0.;
}

std::map<unsigned int, unsigned int>
OneD3EqnEnergyHeatFlux::getVariableIndexMapping() const
{
  std::map<unsigned int, unsigned int> jvar_map;
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhoA_jvar, THM3Eqn::EQ_MASS));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhouA_jvar, THM3Eqn::EQ_MOMENTUM));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhoEA_jvar, THM3Eqn::EQ_ENERGY));
  return jvar_map;
}
