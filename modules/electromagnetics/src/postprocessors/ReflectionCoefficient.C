//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReflectionCoefficient.h"
#include "ElectromagneticConstants.h"
#include <complex>

registerMooseObject("ElectromagneticsApp", ReflectionCoefficient);

InputParameters
ReflectionCoefficient::validParams()
{
  InputParameters params = SidePostprocessor::validParams();
  params.addClassDescription(
      "CURRENTLY ONLY FOR 1D PLANE WAVE SOLVES. Calculate power reflection coefficient "
      "for impinging wave on a surface. Assumes that wave of form F = F_incoming + R*F_reflected");
  params.addRequiredCoupledVar(
      "field_real", "The name of the real field variable this postprocessor operates on.");
  params.addRequiredCoupledVar("field_imag", "Coupled imaginary field variable.");
  params.addRequiredRangeCheckedParam<Real>("theta", "theta>=0", "Wave incidence angle");
  params.addRequiredRangeCheckedParam<Real>("length", "length>0", "Domain length");
  params.addRequiredRangeCheckedParam<Real>("k", "k>0", "Wave number");
  params.addRangeCheckedParam<Real>(
      "incoming_field_magnitude", 1.0, "incoming_field_magnitude>0", "Incoming field magnitude");
  return params;
}

ReflectionCoefficient::ReflectionCoefficient(const InputParameters & parameters)
  : SidePostprocessor(parameters),
    MooseVariableInterface<Real>(this, false, "field_real"),
    _qp(0),
    _coupled_real(coupledValue("field_real")),
    _coupled_imag(coupledValue("field_imag")),
    _theta(getParam<Real>("theta")),
    _length(getParam<Real>("length")),
    _k(getParam<Real>("k")),
    _incoming_mag(getParam<Real>("incoming_field_magnitude"))
{
}

void
ReflectionCoefficient::initialize()
{
  if (_current_elem->dim() > 1)
  {
    mooseError("The ReflectionCoefficient object is not currently configured to work with 2D or 3D "
               "meshes. Please disable this object or setup a 1D mesh!");
  }

  _reflection_coefficient = 0;
}

void
ReflectionCoefficient::execute()
{
  _reflection_coefficient = computeReflection();
}

PostprocessorValue
ReflectionCoefficient::getValue()
{
  return _reflection_coefficient;
}

void
ReflectionCoefficient::threadJoin(const UserObject & y)
{
  const ReflectionCoefficient & pps = static_cast<const ReflectionCoefficient &>(y);
  Real temp_rc = _reflection_coefficient;
  _reflection_coefficient = std::max(temp_rc, pps._reflection_coefficient);
}

void
ReflectionCoefficient::finalize()
{
  gatherMax(_reflection_coefficient);
}

Real
ReflectionCoefficient::computeReflection()
{
  std::complex<double> field(_coupled_real[_qp], _coupled_imag[_qp]);

  std::complex<double> incoming_wave =
      _incoming_mag * std::exp(EM::j * _k * _length * std::cos(_theta * libMesh::pi / 180.));
  std::complex<double> reversed_wave =
      _incoming_mag * std::exp(-EM::j * _k * _length * std::cos(_theta * libMesh::pi / 180.));

  std::complex<double> reflection_coefficient_complex = (field - incoming_wave) / reversed_wave;

  return std::pow(std::abs(reflection_coefficient_complex), 2);
}
