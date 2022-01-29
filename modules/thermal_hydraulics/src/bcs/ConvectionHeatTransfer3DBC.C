#include "ConvectionHeatTransfer3DBC.h"
#include "HeatTransferFromHeatStructure3D1PhaseUserObject.h"
#include "THMIndices3Eqn.h"
#include "Assembly.h"
#include "NonlinearSystemBase.h"

registerMooseObject("ThermalHydraulicsApp", ConvectionHeatTransfer3DBC);

InputParameters
ConvectionHeatTransfer3DBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<UserObjectName>(
      "ht_uo", "The name of the user object that computes the heat flux");
  params.addRequiredCoupledVar("rhoA", "rho*A of the flow channel");
  params.addRequiredCoupledVar("rhouA", "rhou*A of the flow channel");
  params.addRequiredCoupledVar("rhoEA", "rhoE*A of the flow channel");
  params.addRequiredCoupledVar("T_wall", "Wall temperature of the flow channel");
  return params;
}

ConvectionHeatTransfer3DBC::ConvectionHeatTransfer3DBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _ht_uo(getUserObject<HeatTransferFromHeatStructure3D1PhaseUserObject>("ht_uo")),
    _rhoA_jvar(coupled("rhoA")),
    _rhouA_jvar(coupled("rhouA")),
    _rhoEA_jvar(coupled("rhoEA")),
    _T_wall_jvar(coupled("T_wall")),
    _jvar_map(getVariableIndexMapping()),
    _phi_neighbor(_assembly.phiNeighbor(_var))
{
}

void
ConvectionHeatTransfer3DBC::initialSetup()
{
  _off_diag_var_nums = getOffDiagVariableNumbers();
}

std::vector<unsigned int>
ConvectionHeatTransfer3DBC::getOffDiagVariableNumbers()
{
  return {_rhoA_jvar, _rhouA_jvar, _rhoEA_jvar};
}

Real
ConvectionHeatTransfer3DBC::computeQpResidual()
{
  const std::vector<Real> & T_fluid = _ht_uo.getTfluid(_current_elem->id());
  const std::vector<Real> & htc = _ht_uo.getHeatTransferCoeff(_current_elem->id());
  return htc[_qp] * (_u[_qp] - T_fluid[_qp]) * _test[_i][_qp];
}

Real
ConvectionHeatTransfer3DBC::computeQpJacobian()
{
  const std::vector<Real> & htc = _ht_uo.getHeatTransferCoeff(_current_elem->id());
  return htc[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}

Real
ConvectionHeatTransfer3DBC::computeQpOffDiagJacobianNeighbor(unsigned int jvar)
{
  std::map<unsigned int, unsigned int>::const_iterator it;
  if ((it = _jvar_map.find(jvar)) != _jvar_map.end())
  {
    const std::vector<DenseVector<Real>> & dT_fluid = _ht_uo.getTfluidJacobian(_current_elem->id());
    const std::vector<Real> & htc = _ht_uo.getHeatTransferCoeff(_current_elem->id());
    return -htc[_qp] * dT_fluid[_qp](it->second) * _phi_neighbor[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0.;
}

void
ConvectionHeatTransfer3DBC::computeOffDiagJacobian(const unsigned int jvar_num)
{
  IntegratedBC::computeOffDiagJacobian(jvar_num);

  if (jvar_num == _var.number())
  {
    // when doing the diagonal part, also take care of the off-diag jacobian
    // wrt the heat structure side
    std::vector<dof_id_type> idofs = _var.dofIndices();

    const dof_id_type & pipe_elem_id = _ht_uo.getNearestElem(_current_elem->id());
    const Elem * neighbor = _mesh.elemPtr(pipe_elem_id);

    _assembly.setCurrentNeighborSubdomainID(neighbor->subdomain_id());
    _assembly.reinitNeighborAtPhysical(neighbor, _q_point.stdVector());

    for (std::size_t i = 0; i < _off_diag_var_nums.size(); i++)
    {
      unsigned int jvar_num = _off_diag_var_nums[i];
      MooseVariableFEBase & jvar = _fe_problem.getNonlinearSystemBase().getVariable(_tid, jvar_num);
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

std::map<unsigned int, unsigned int>
ConvectionHeatTransfer3DBC::getVariableIndexMapping() const
{
  std::map<unsigned int, unsigned int> jvar_map;
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhoA_jvar, THM3Eqn::EQ_MASS));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhouA_jvar, THM3Eqn::EQ_MOMENTUM));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_rhoEA_jvar, THM3Eqn::EQ_ENERGY));
  jvar_map.insert(std::pair<unsigned int, unsigned int>(_T_wall_jvar, THM3Eqn::EQ_ENERGY + 1));
  return jvar_map;
}
