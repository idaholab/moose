//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComplexHeating.h"
#include "Assembly.h"
#include "ElectromagneticEnums.h"
#include "ElectromagneticConstants.h"
#include "Function.h"
#include <complex>

registerMooseObject("ElectromagneticsApp", ComplexHeating);

InputParameters
ComplexHeating::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Supplies the heating due to the electic field in the "
  "form of $(0.5Re(\\sigma E \\cdot E^{*} ))$");
  params.addRequiredCoupledVar("E_real", "The real component of the E-field.");
  params.addRequiredCoupledVar("E_imag", "The imaginary component of the E-field.");
  params.addRequiredParam<std::string>("conductivity", "The real component of the material conductivity.");
  params.addParam<Real>("value", 1.0, "Coefficient to multiply by heating term.");
  return params;
}

ComplexHeating::ComplexHeating(const InputParameters & parameters)
  : ADKernel(parameters),
    _E_real(adCoupledVectorValue("E_real")),
    _E_imag(adCoupledVectorValue("E_imag")),
    _cond(getADMaterialProperty<Real>(getParam<std::string>("conductivity"))),
    _scale(getParam<Real>("value"))
{
}

ADReal
ComplexHeating::computeQpResidual()
{
  return -_test[_i][_qp] * _scale * 0.5 * _cond[_qp] *
         (_E_real[_qp] * _E_real[_qp] + _E_imag[_qp] * _E_imag[_qp]);
}
