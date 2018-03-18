//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EntropyFromEnthalpyPressureDerivativesTestKernel.h"

registerMooseObject("FluidPropertiesTestApp", EntropyFromEnthalpyPressureDerivativesTestKernel);

template <>
InputParameters
validParams<EntropyFromEnthalpyPressureDerivativesTestKernel>()
{
  InputParameters params = validParams<FluidPropertyDerivativesTestKernel>();

  params.addClassDescription(
      "Tests derivatives of specific entropy from specific enthalpy and pressure");

  params.addRequiredCoupledVar("h", "Specific enthalpy");
  params.addRequiredCoupledVar("p", "Pressure");

  return params;
}

EntropyFromEnthalpyPressureDerivativesTestKernel::EntropyFromEnthalpyPressureDerivativesTestKernel(
    const InputParameters & parameters)
  : FluidPropertyDerivativesTestKernel(parameters),
    _h(coupledValue("h")),
    _p(coupledValue("p")),
    _h_index(coupled("h")),
    _p_index(coupled("p"))
{
}

EntropyFromEnthalpyPressureDerivativesTestKernel::
    ~EntropyFromEnthalpyPressureDerivativesTestKernel()
{
}

Real
EntropyFromEnthalpyPressureDerivativesTestKernel::computeQpResidual()
{
  Real s, ds_dh, ds_dp;
  _fp.s_from_h_p(_h[_qp], _p[_qp], s, ds_dh, ds_dp);

  return s;
}

Real
EntropyFromEnthalpyPressureDerivativesTestKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real s, ds_dh, ds_dp;
  _fp.s_from_h_p(_h[_qp], _p[_qp], s, ds_dh, ds_dp);

  if (jvar == _h_index)
    return ds_dh * _phi[_j][_qp];
  else if (jvar == _p_index)
    return ds_dp * _phi[_j][_qp];
  else
    return 0;
}
