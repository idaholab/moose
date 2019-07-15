#include "OneD3EqnEnergyHeatFlux.h"
#include "HeatFluxFromHeatStructureBaseUserObject.h"
#include "THMIndices3Eqn.h"
#include "FlowModelSinglePhase.h"
#include "HeatConductionModel.h"

registerMooseObject("THMApp", OneD3EqnEnergyHeatFlux);

template <>
InputParameters
validParams<OneD3EqnEnergyHeatFlux>()
{
  InputParameters params = validParams<OneDHeatFluxBase>();
  params.addRequiredCoupledVar("rhoA", "rho*A of the flow channel");
  params.addRequiredCoupledVar("rhouA", "rhou*A of the flow channel");
  params.addRequiredCoupledVar("rhoEA", "rhoE*A of the flow channel");
  return params;
}

OneD3EqnEnergyHeatFlux::OneD3EqnEnergyHeatFlux(const InputParameters & parameters)
  : OneDHeatFluxBase(parameters),
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
  const std::vector<Real> & P_hf = _q_uo.getHeatedPerimeter(_current_elem->id());
  return -q_wall[_qp] * P_hf[_qp] * _test[_i][_qp];
}

Real
OneD3EqnEnergyHeatFlux::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
OneD3EqnEnergyHeatFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  std::map<unsigned int, unsigned int>::const_iterator it;
  if ((it = _jvar_map.find(jvar)) != _jvar_map.end())
  {
    const std::vector<DenseVector<Real>> & dq_wall = _q_uo.getHeatFluxJacobian(_current_elem->id());
    const std::vector<Real> & P_hf = _q_uo.getHeatedPerimeter(_current_elem->id());
    return -dq_wall[_qp](it->second) * P_hf[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0.;
}

Real
OneD3EqnEnergyHeatFlux::computeQpOffDiagJacobianNeighbor(unsigned int jvar)
{
  std::map<unsigned int, unsigned int>::const_iterator it;
  if ((it = _jvar_map.find(jvar)) != _jvar_map.end())
  {
    const std::vector<DenseVector<Real>> & dq_wall = _q_uo.getHeatFluxJacobian(_current_elem->id());
    const std::vector<Real> & P_hf = _q_uo.getHeatedPerimeter(_current_elem->id());
    return -dq_wall[_qp](it->second) * P_hf[_qp] * _phi_neighbor[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0.;
}

std::map<unsigned int, unsigned int>
OneD3EqnEnergyHeatFlux::getVariableIndexMapping() const
{
  std::map<unsigned int, unsigned int> jvar_map;
  std::vector<std::string> var_names = {FlowModelSinglePhase::RHOA,
                                        FlowModelSinglePhase::RHOUA,
                                        FlowModelSinglePhase::RHOEA,
                                        HeatConductionModel::TEMPERATURE};
  for (std::size_t i = 0; i < var_names.size(); i++)
  {
    MooseVariableFEBase & jvar = _fe_problem.getVariable(_tid, var_names[i]);
    jvar_map.insert(std::pair<unsigned int, unsigned int>(jvar.number(), i));
  }
  return jvar_map;
}
