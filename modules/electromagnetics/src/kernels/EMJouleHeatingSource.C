//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EMJouleHeatingSource.h"
#include "Assembly.h"
#include "ElectromagneticEnums.h"
#include "ElectromagneticConstants.h"
#include "Function.h"
#include <complex>

registerMooseObject("ElectromagneticsApp", EMJouleHeatingSource);

InputParameters
EMJouleHeatingSource::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Supplies the heating due to the electic field in the "
                             "form of $(0.5Re(\\sigma E \\cdot E^{*} ))$");
  params.addRequiredCoupledVar("E_real", "The real component of the electric field.");
  params.addRequiredCoupledVar("E_imag", "The imaginary component of the electric field.");
  params.addRequiredParam<std::string>("conductivity",
                                       "The real component of the material conductivity.");
  params.addParam<Real>("value", 1.0, "Coefficient to multiply by heating term.");
  return params;
}

EMJouleHeatingSource::EMJouleHeatingSource(const InputParameters & parameters)
  : ADKernel(parameters),
    _E_real(adCoupledVectorValue("E_real")),
    _E_imag(adCoupledVectorValue("E_imag")),
    _cond(getADMaterialProperty<Real>(getParam<std::string>("conductivity"))),
    _scale(getParam<Real>("value"))
{
  mooseDeprecated("This kernel will be deprecated in the near future (10/01/2025) in favor of "
                  "exclusively using the Heat Transfer module's 'ADJouleHeatingSource' for "
                  "coupling electromagnetics to heat transfer problems.");
}

ADReal
EMJouleHeatingSource::computeQpResidual()
{
  return -_test[_i][_qp] * _scale * 0.5 * _cond[_qp] *
         (_E_real[_qp] * _E_real[_qp] + _E_imag[_qp] * _E_imag[_qp]);
}
