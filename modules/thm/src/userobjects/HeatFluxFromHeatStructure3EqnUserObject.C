#include "HeatFluxFromHeatStructure3EqnUserObject.h"
#include "THMIndices3Eqn.h"
#include "FlowModelSinglePhase.h"
#include "HeatConductionModel.h"

registerMooseObject("THMApp", HeatFluxFromHeatStructure3EqnUserObject);

template <>
InputParameters
validParams<HeatFluxFromHeatStructure3EqnUserObject>()
{
  InputParameters params = validParams<HeatFluxFromHeatStructureBaseUserObject>();
  params.addRequiredCoupledVar("T_wall", "Wall temperature");
  params.addRequiredParam<MaterialPropertyName>("T", "Fluid temperature");
  params.addRequiredParam<MaterialPropertyName>("Hw", "Convective heat transfer coefficient");
  params.addRequiredCoupledVar("P_hf", "Heat flux perimeter");
  params.addRequiredCoupledVar("rhoA", "rho*A of the flow channel");
  params.addRequiredCoupledVar("rhouA", "rhou*A of the flow channel");
  params.addRequiredCoupledVar("rhoEA", "rhoE*A of the flow channel");
  params.addClassDescription(
      "Cache the heat flux between a single phase flow channel and a heat structure");
  return params;
}

HeatFluxFromHeatStructure3EqnUserObject::HeatFluxFromHeatStructure3EqnUserObject(
    const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<HeatFluxFromHeatStructureBaseUserObject>(parameters),
    _T_wall(coupledValue("T_wall")),
    _P_hf(coupledValue("P_hf")),
    _Hw(getMaterialProperty<Real>("Hw")),
    _T(getMaterialProperty<Real>("T")),
    _dT_drhoA(getMaterialPropertyDerivativeTHM<Real>("T", "rhoA")),
    _dT_drhouA(getMaterialPropertyDerivativeTHM<Real>("T", "rhouA")),
    _dT_drhoEA(getMaterialPropertyDerivativeTHM<Real>("T", "rhoEA"))
{
}

void
HeatFluxFromHeatStructure3EqnUserObject::execute()
{
  dof_id_type elem_id = _current_elem->id();

  // 1D elements
  _dofs[elem_id].resize(THM3Eqn::N_EQ);
  std::vector<std::string> var_names = {
      FlowModelSinglePhase::RHOA, FlowModelSinglePhase::RHOUA, FlowModelSinglePhase::RHOEA};
  for (std::size_t i = 0; i < var_names.size(); i++)
  {
    MooseVariable & var = _fe_problem.getStandardVariable(_tid, var_names[i]);
    std::vector<dof_id_type> dofs;
    var.getDofIndices(_current_elem, dofs);
    _dofs[elem_id][i] = dofs;
  }

  dof_id_type nearest_elem_id = _nearest_elem_ids[elem_id];
  const Elem * nearest_elem = _mesh.elemPtr(nearest_elem_id);
  _dofs[nearest_elem_id].resize(1);
  {
    MooseVariable & var = _fe_problem.getStandardVariable(_tid, HeatConductionModel::TEMPERATURE);
    std::vector<dof_id_type> dofs;
    var.getDofIndices(nearest_elem, dofs);
    _dofs[nearest_elem_id][0] = dofs;
  }

  HeatFluxFromHeatStructureBaseUserObject::execute();
}

Real
HeatFluxFromHeatStructure3EqnUserObject::computeQpHeatFlux()
{
  return _Hw[_qp] * (_T_wall[_qp] - _T[_qp]) * _P_hf[_qp];
}

DenseVector<Real>
HeatFluxFromHeatStructure3EqnUserObject::computeQpHeatFluxJacobian()
{
  DenseVector<Real> jac(4);
  jac(THM3Eqn::EQ_MASS) = -_Hw[_qp] * _dT_drhoA[_qp] * _P_hf[_qp];
  jac(THM3Eqn::EQ_MOMENTUM) = -_Hw[_qp] * _dT_drhouA[_qp] * _P_hf[_qp];
  jac(THM3Eqn::EQ_ENERGY) = -_Hw[_qp] * _dT_drhoEA[_qp] * _P_hf[_qp];
  jac(THM3Eqn::EQ_ENERGY + 1) = _Hw[_qp] * _P_hf[_qp];
  return jac;
}
