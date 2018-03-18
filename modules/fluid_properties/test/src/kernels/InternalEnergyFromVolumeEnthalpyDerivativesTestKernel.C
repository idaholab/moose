//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalEnergyFromVolumeEnthalpyDerivativesTestKernel.h"

registerMooseObject("FluidPropertiesTestApp",
                    InternalEnergyFromVolumeEnthalpyDerivativesTestKernel);

template <>
InputParameters
validParams<InternalEnergyFromVolumeEnthalpyDerivativesTestKernel>()
{
  InputParameters params = validParams<FluidPropertyDerivativesTestKernel>();

  params.addClassDescription(
      "Tests derivatives of specific internal energy from specific volume and specific enthalpy");

  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredCoupledVar("h", "Specific enthalpy");

  return params;
}

InternalEnergyFromVolumeEnthalpyDerivativesTestKernel::
    InternalEnergyFromVolumeEnthalpyDerivativesTestKernel(const InputParameters & parameters)
  : FluidPropertyDerivativesTestKernel(parameters),
    _v(coupledValue("v")),
    _h(coupledValue("h")),
    _v_index(coupled("v")),
    _h_index(coupled("h"))
{
}

InternalEnergyFromVolumeEnthalpyDerivativesTestKernel::
    ~InternalEnergyFromVolumeEnthalpyDerivativesTestKernel()
{
}

Real
InternalEnergyFromVolumeEnthalpyDerivativesTestKernel::computeQpResidual()
{
  return _fp.e_from_v_h(_v[_qp], _h[_qp]);
}

Real
InternalEnergyFromVolumeEnthalpyDerivativesTestKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real e, de_dv, de_dh;
  _fp.e_from_v_h(_v[_qp], _h[_qp], e, de_dv, de_dh);

  if (jvar == _v_index)
    return de_dv * _phi[_j][_qp];
  else if (jvar == _h_index)
    return de_dh * _phi[_j][_qp];
  else
    return 0;
}
