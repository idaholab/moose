//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PFCEnergyDensity.h"
#include "libmesh/utility.h"

registerMooseObject("PhaseFieldApp", PFCEnergyDensity);

InputParameters
PFCEnergyDensity::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes the crystal free energy density");
  params.addRequiredCoupledVar("v", "Array of coupled variables");
  return params;
}

PFCEnergyDensity::PFCEnergyDensity(const InputParameters & parameters)
  : AuxKernel(parameters),
    _vals(coupledValues("v")),
    _order(coupledComponents("v")),
    _a(getMaterialProperty<Real>("a")),
    _b(getMaterialProperty<Real>("b"))
{
  _coeff.resize(_order);

  std::string coeff_name_base = "C";

  for (unsigned int i = 0; i < _order; ++i)
  {
    std::string coeff_name = coeff_name_base;
    std::stringstream out;
    out << i * 2;
    coeff_name.append(out.str());
    _console << coeff_name << std::endl;
    _coeff[i] = &getMaterialProperty<Real>(coeff_name);
  }
}

Real
PFCEnergyDensity::computeValue()
{
  Real val = Utility::pow<2>((*_vals[0])[_qp]) * (1.0 - (*_coeff[0])[_qp]) / 2.0;

  // Loop Through Variables
  // the sign of negative terms have been taken care of by changing the sign of the coefficients;
  for (unsigned int i = 1; i < _order; ++i)
    val += (*_coeff[i])[_qp] * (*_vals[0])[_qp] * (*_vals[i])[_qp] / 2.0;

  val += (_b[_qp] / 12.0 * Utility::pow<4>((*_vals[0])[_qp])) -
         (_a[_qp] / 6.0 * Utility::pow<3>((*_vals[0])[_qp]));

  return val;
}
