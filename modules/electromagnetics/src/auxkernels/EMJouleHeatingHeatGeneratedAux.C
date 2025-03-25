//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EMJouleHeatingHeatGeneratedAux.h"

registerMooseObject("ElectromagneticsApp", EMJouleHeatingHeatGeneratedAux);

InputParameters
EMJouleHeatingHeatGeneratedAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes the heating due to the electic field in the "
                             "form of $(0.5Re(\\sigma E \\cdot E^{*} ))$");
  params.addRequiredCoupledVar("E_real", "The real component of the electric field.");
  params.addRequiredCoupledVar("E_imag", "The imaginary component of the electric field.");
  params.addRequiredParam<std::string>("conductivity",
                                       "The real component of the material conductivity.");
  return params;
}

EMJouleHeatingHeatGeneratedAux::EMJouleHeatingHeatGeneratedAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _E_real(coupledVectorValue("E_real")),
    _E_imag(coupledVectorValue("E_imag")),
    _cond(getADMaterialProperty<Real>(getParam<std::string>("conductivity")))
{
}

Real
EMJouleHeatingHeatGeneratedAux::computeValue()
{
  return 0.5 * raw_value(_cond[_qp]) * (_E_real[_qp] * _E_real[_qp] + _E_imag[_qp] * _E_imag[_qp]);
}
