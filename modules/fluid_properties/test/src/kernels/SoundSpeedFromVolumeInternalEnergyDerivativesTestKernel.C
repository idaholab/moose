//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel.h"

registerMooseObject("FluidPropertiesTestApp",
                    SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel);

template <>
InputParameters
validParams<SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel>()
{
  InputParameters params = validParams<FluidPropertyDerivativesTestKernel>();

  params.addClassDescription(
      "Tests derivatives of sound speed from specific volume and specific internal energy");

  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredCoupledVar("e", "Specific internal energy");

  return params;
}

SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel::
    SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel(const InputParameters & parameters)
  : FluidPropertyDerivativesTestKernel(parameters),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _v_index(coupled("v")),
    _e_index(coupled("e"))
{
}

SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel::
    ~SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel()
{
}

Real
SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel::computeQpResidual()
{
  return _fp.c_from_v_e(_v[_qp], _e[_qp]);
}

Real
SoundSpeedFromVolumeInternalEnergyDerivativesTestKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real c, dc_dv, dc_de;
  _fp.c_from_v_e(_v[_qp], _e[_qp], c, dc_dv, dc_de);

  if (jvar == _v_index)
    return dc_dv * _phi[_j][_qp];
  else if (jvar == _e_index)
    return dc_de * _phi[_j][_qp];
  else
    return 0;
}
