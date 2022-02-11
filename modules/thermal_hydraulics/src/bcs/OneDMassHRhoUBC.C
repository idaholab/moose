//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDMassHRhoUBC.h"

registerMooseObject("ThermalHydraulicsApp", OneDMassHRhoUBC);

InputParameters
OneDMassHRhoUBC::validParams()
{
  InputParameters params = OneDIntegratedBC::validParams();
  params.addRequiredParam<Real>("rhou", "Specified momentum");
  params.addRequiredCoupledVar("A", "Area");

  params.declareControllable("rhou");

  return params;
}

OneDMassHRhoUBC::OneDMassHRhoUBC(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _rhou(getParam<Real>("rhou")),
    _area(coupledValue("A"))
{
}

Real
OneDMassHRhoUBC::computeQpResidual()
{
  return _rhou * _area[_qp] * _normal * _test[_i][_qp];
}

Real
OneDMassHRhoUBC::computeQpJacobian()
{
  return 0;
}

Real
OneDMassHRhoUBC::computeQpOffDiagJacobian(unsigned int)
{
  return 0;
}
