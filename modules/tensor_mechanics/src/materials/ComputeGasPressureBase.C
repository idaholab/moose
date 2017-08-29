/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeGasPressureBase.h"

template <>
InputParameters
validParams<ComputeGasPressureBase>()
{
  InputParameters params = validParams<ComputeExtraStressBase>();
  params.addClassDescription("Computes a constant extra stress that is added to the stress "
                             "calculated by the constitutive model");
  params.addRequiredCoupledVar("c", "Gas concentration");
  params.addRequiredCoupledVar("T", "Temperatue");
  params.addRequiredParam<Real>(
      "omega", "Lattice site volume (default mass_unit_conversion requires this to be in [Ang^3])");
  params.addParam<Real>("pressure_unit_conversion",
                        160217.66208,
                        "Conversion factor for the pressure (defaults to "
                        "[MPa/(eV/Ang^3)] to obtain pressure in [MPa])");
  params.addParam<Real>("kB", 8.6173303e-5, "Boltzmann constant (default in [eV/K])");
  return params;
}

ComputeGasPressureBase::ComputeGasPressureBase(const InputParameters & parameters)
  : ComputeExtraStressBase(parameters),
    _c(coupledValue("c")),
    _T(coupledValue("T")),
    _omega(getParam<Real>("omega")),
    _pressure_unit_conversion(getParam<Real>("pressure_unit_conversion")),
    _kB(getParam<Real>("kB"))
{
}
