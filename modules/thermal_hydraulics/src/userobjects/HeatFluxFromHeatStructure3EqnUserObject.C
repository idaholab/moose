//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatFluxFromHeatStructure3EqnUserObject.h"
#include "THMIndices3Eqn.h"
#include "FlowModelSinglePhase.h"
#include "HeatConductionModel.h"

registerMooseObject("ThermalHydraulicsApp", HeatFluxFromHeatStructure3EqnUserObject);

InputParameters
HeatFluxFromHeatStructure3EqnUserObject::validParams()
{
  InputParameters params = HeatFluxFromHeatStructureBaseUserObject::validParams();
  params.addRequiredCoupledVar("T_wall", "Wall temperature");
  params.addRequiredParam<MaterialPropertyName>("T", "Fluid temperature");
  params.addRequiredParam<MaterialPropertyName>("Hw", "Convective heat transfer coefficient");
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
  return _Hw[_qp] * (_T_wall[_qp] - _T[_qp]);
}

DenseVector<Real>
HeatFluxFromHeatStructure3EqnUserObject::computeQpHeatFluxJacobian()
{
  DenseVector<Real> jac(4);
  jac(THM3Eqn::EQ_MASS) = -_Hw[_qp] * _dT_drhoA[_qp];
  jac(THM3Eqn::EQ_MOMENTUM) = -_Hw[_qp] * _dT_drhouA[_qp];
  jac(THM3Eqn::EQ_ENERGY) = -_Hw[_qp] * _dT_drhoEA[_qp];
  jac(THM3Eqn::EQ_ENERGY + 1) = _Hw[_qp];
  return jac;
}
