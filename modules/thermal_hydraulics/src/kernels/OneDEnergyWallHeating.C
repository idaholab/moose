//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDEnergyWallHeating.h"

registerMooseObject("ThermalHydraulicsApp", OneDEnergyWallHeating);

InputParameters
OneDEnergyWallHeating::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar("rhoA", "");
  params.addRequiredCoupledVar("rhouA", "");
  params.addRequiredCoupledVar("rhoEA", "Energy equation variable");
  params.addRequiredCoupledVar("P_hf", "heat flux perimeter");
  params.addCoupledVar("T_wall", 0, "Wall temperature (const)");
  params.addRequiredParam<MaterialPropertyName>("Hw",
                                                "Convective heat transfer coefficient, W/m^2-K");
  params.addRequiredParam<MaterialPropertyName>("T", "Temperature material property");

  return params;
}

OneDEnergyWallHeating::OneDEnergyWallHeating(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _temperature(getMaterialProperty<Real>("T")),
    _dT_drhoA(getMaterialPropertyDerivativeTHM<Real>("T", "rhoA")),
    _dT_drhouA(getMaterialPropertyDerivativeTHM<Real>("T", "rhouA")),
    _dT_drhoEA(getMaterialPropertyDerivativeTHM<Real>("T", "rhoEA")),
    _Hw(getMaterialProperty<Real>("Hw")),
    _T_wall(coupledValue("T_wall")),
    _P_hf(coupledValue("P_hf")),
    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA"))
{
}

Real
OneDEnergyWallHeating::computeQpResidual()
{
  return _Hw[_qp] * _P_hf[_qp] * (_temperature[_qp] - _T_wall[_qp]) * _test[_i][_qp];
}

Real
OneDEnergyWallHeating::computeQpJacobian()
{
  return _Hw[_qp] * _P_hf[_qp] * _dT_drhoEA[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}

Real
OneDEnergyWallHeating::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _rhoA_var_number)
    return _Hw[_qp] * _P_hf[_qp] * _dT_drhoA[_qp] * _phi[_j][_qp] * _test[_i][_qp];

  else if (jvar == _rhouA_var_number)
    return _Hw[_qp] * _P_hf[_qp] * _dT_drhouA[_qp] * _phi[_j][_qp] * _test[_i][_qp];

  else
    return 0.;
}
