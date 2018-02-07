//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SoundSpeedFromVolumeEnthalpyDerivativesTestKernel.h"

template <>
InputParameters
validParams<SoundSpeedFromVolumeEnthalpyDerivativesTestKernel>()
{
  InputParameters params = validParams<FluidPropertyDerivativesTestKernel>();

  params.addClassDescription(
      "Tests derivatives of sound speed from specific volume and specific enthalpy");

  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredCoupledVar("h", "Specific enthalpy");

  return params;
}

SoundSpeedFromVolumeEnthalpyDerivativesTestKernel::
    SoundSpeedFromVolumeEnthalpyDerivativesTestKernel(const InputParameters & parameters)
  : FluidPropertyDerivativesTestKernel(parameters),
    _v(coupledValue("v")),
    _h(coupledValue("h")),
    _v_index(coupled("v")),
    _h_index(coupled("h"))
{
}

SoundSpeedFromVolumeEnthalpyDerivativesTestKernel::
    ~SoundSpeedFromVolumeEnthalpyDerivativesTestKernel()
{
}

Real
SoundSpeedFromVolumeEnthalpyDerivativesTestKernel::computeQpResidual()
{
  return _fp.c_from_v_h(_v[_qp], _h[_qp]);
}

Real
SoundSpeedFromVolumeEnthalpyDerivativesTestKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real c, dc_dv, dc_dh;
  _fp.c_from_v_h(_v[_qp], _h[_qp], c, dc_dv, dc_dh);

  if (jvar == _v_index)
    return dc_dv * _phi[_j][_qp];
  else if (jvar == _h_index)
    return dc_dh * _phi[_j][_qp];
  else
    return 0;
}
