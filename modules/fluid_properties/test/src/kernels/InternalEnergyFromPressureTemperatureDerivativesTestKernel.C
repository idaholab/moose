//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalEnergyFromPressureTemperatureDerivativesTestKernel.h"

registerMooseObject("FluidPropertiesTestApp",
                    InternalEnergyFromPressureTemperatureDerivativesTestKernel);

template <>
InputParameters
validParams<InternalEnergyFromPressureTemperatureDerivativesTestKernel>()
{
  InputParameters params = validParams<FluidPropertyDerivativesTestKernel>();

  params.addClassDescription(
      "Tests derivatives of internal energy from pressure and temperature");

  params.addRequiredCoupledVar("p", "Pressure");
  params.addRequiredCoupledVar("T", "Temperature");

  return params;
}

InternalEnergyFromPressureTemperatureDerivativesTestKernel::
    InternalEnergyFromPressureTemperatureDerivativesTestKernel(const InputParameters & parameters)
  : FluidPropertyDerivativesTestKernel(parameters),
    _p(coupledValue("p")),
    _T(coupledValue("T")),
    _p_index(coupled("p")),
    _T_index(coupled("T"))
{
}

InternalEnergyFromPressureTemperatureDerivativesTestKernel::
    ~InternalEnergyFromPressureTemperatureDerivativesTestKernel()
{
}

Real
InternalEnergyFromPressureTemperatureDerivativesTestKernel::computeQpResidual()
{
  return _fp.e_from_p_T(_p[_qp], _T[_qp]);
}

Real
InternalEnergyFromPressureTemperatureDerivativesTestKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real e, de_dp, de_dT;
  _fp.e_from_p_T(_p[_qp], _T[_qp], e, de_dp, de_dT);

  if (jvar == _p_index)
    return de_dp * _phi[_j][_qp];
  else if (jvar == _T_index)
    return de_dT * _phi[_j][_qp];
  else
    return 0;
}
