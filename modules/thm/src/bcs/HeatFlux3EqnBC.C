#include "HeatFlux3EqnBC.h"
#include "HeatFluxFromHeatStructureBaseUserObject.h"
#include "THMIndices3Eqn.h"

registerMooseObject("THMApp", HeatFlux3EqnBC);

template <>
InputParameters
validParams<HeatFlux3EqnBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<UserObjectName>(
      "q_uo", "The name of the user object that computes the heat flux");
  params.addRequiredParam<Real>("P_hs_unit", "Perimeter of a single unit of heat structure");
  params.addRequiredParam<unsigned int>("n_unit", "Number of units of heat structure");
  params.addRequiredParam<bool>("hs_coord_system_is_cylindrical",
                                "Is the heat structure coordinate system cylindrical?");
  params.addRequiredCoupledVar("rhoA", "rho*A of the flow channel");
  params.addRequiredCoupledVar("rhouA", "rhou*A of the flow channel");
  params.addRequiredCoupledVar("rhoEA", "rhoE*A of the flow channel");
  params.addRequiredCoupledVar("T_wall", "Wall temperature of the flow channel");
  return params;
}

HeatFlux3EqnBC::HeatFlux3EqnBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _q_uo(getUserObject<HeatFluxFromHeatStructureBaseUserObject>("q_uo")),
    _P_hs_unit(getParam<Real>("P_hs_unit")),
    _n_unit(getParam<unsigned int>("n_unit")),
    _hs_coord_system_is_cylindrical(getParam<bool>("hs_coord_system_is_cylindrical")),
    _hs_coord(_hs_coord_system_is_cylindrical ? _P_hs_unit : 1.0),
    _hs_scale(-_hs_coord / (_n_unit * _P_hs_unit)),
    _rhoA_jvar(coupled("rhoA")),
    _rhouA_jvar(coupled("rhouA")),
    _rhoEA_jvar(coupled("rhoEA")),
    _T_wall_jvar(coupled("T_wall")),
    _jvar_map(getVariableIndexMapping())
{
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
HeatFlux3EqnBC::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0;
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
