#include "OneD3EqnEnergyHeatFlux.h"
#include "HeatFluxFromHeatStructure3EqnUserObject.h"
#include "THMIndices3Eqn.h"
#include "FlowModelSinglePhase.h"
#include "HeatConductionModel.h"
#include "Assembly.h"

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
    _phi_neighbor(_assembly.phiNeighbor(_var)),
    _q_uo(getUserObject<HeatFluxFromHeatStructure3EqnUserObject>("q_uo")),
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

void
OneD3EqnEnergyHeatFlux::computeJacobian()
{
  Kernel::computeJacobian();
}

void
OneD3EqnEnergyHeatFlux::computeOffDiagJacobian(unsigned jvar)
{
  Kernel::computeOffDiagJacobian(jvar);
}

void
OneD3EqnEnergyHeatFlux::computeOffDiagJacobian(MooseVariableFEBase & jvar)
{
  Kernel::computeOffDiagJacobian(jvar);

  if (jvar.number() == _var.number())
  {
    // when doing the diagonal part, also take care of the off-diag jacobian
    // wrt the heat structure side
    std::vector<dof_id_type> idofs = _var.dofIndices();

    const dof_id_type & hs_elem_id = _q_uo.getNearestElem(_current_elem->id());
    const Elem * neighbor = _mesh.elemPtr(hs_elem_id);

    _assembly.setCurrentNeighborSubdomainID(neighbor->subdomain_id());
    _assembly.reinitNeighborAtPhysical(neighbor, _q_point.stdVector());

    std::vector<std::string> var_names = {HeatConductionModel::TEMPERATURE};
    for (std::size_t i = 0; i < var_names.size(); i++)
    {
      MooseVariableFEBase & jvar = _fe_problem.getVariable(_tid, var_names[i]);
      unsigned int jvar_num = jvar.number();
      jvar.prepareNeighbor();
      _assembly.copyNeighborShapes(jvar_num);

      auto & jdofs = jvar.dofIndicesNeighbor();
      DenseMatrix<Number> Ke(_test.size(), jvar.phiNeighborSize());
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        for (_i = 0; _i < _test.size(); _i++)
          for (_j = 0; _j < jvar.phiNeighborSize(); _j++)
            Ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobianNeighbor(jvar_num);

      _assembly.cacheJacobianBlock(Ke, idofs, jdofs, _var.scalingFactor());
    }
  }
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

Real
OneD3EqnEnergyHeatFlux::computeQpOffDiagJacobianNeighbor(unsigned int jvar)
{
  auto it = _jvar_map.find(jvar);
  if (it != _jvar_map.end())
  {
    const std::vector<DenseVector<Real>> & dq_wall = _q_uo.getHeatFluxJacobian(_current_elem->id());
    return -dq_wall[_qp](it->second) * _phi_neighbor[_j][_qp] * _test[_i][_qp];
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
