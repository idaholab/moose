#include "HeatFluxFromHeatStructure3EqnUserObject.h"
#include "THMIndices3Eqn.h"

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
