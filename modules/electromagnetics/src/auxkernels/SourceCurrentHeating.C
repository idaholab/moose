//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SourceCurrentHeating.h"
#include "Function.h"

registerMooseObject("ElectromagneticsApp", SourceCurrentHeating);

InputParameters
SourceCurrentHeating::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(("Computes the heating due to the electic field in the "
                              "form of $(0.5Re(J \\cdot E^{*} ))$"));
  params.addRequiredCoupledVar("E_real", "The real component of the electric field.");
  params.addRequiredCoupledVar("E_imag", "The imaginary component of the electric field.");
  params.addRequiredParam<FunctionName>("source_real", "Current Source vector, real component");
  params.addRequiredParam<FunctionName>("source_imag",
                                        "Current Source vector, imaginary component");
  return params;
}

SourceCurrentHeating::SourceCurrentHeating(const InputParameters & parameters)
  : AuxKernel(parameters),
    _E_real(coupledVectorValue("E_real")),
    _E_imag(coupledVectorValue("E_imag")),
    _source_real(getFunction("source_real")),
    _source_imag(getFunction("source_imag"))
{
}

Real
SourceCurrentHeating::computeValue()
{
  return 0.5 * (_source_real.vectorValue(_t, _q_point[_qp]) * _E_real[_qp] +
                _source_imag.vectorValue(_t, _q_point[_qp]) * _E_imag[_qp]);
}
