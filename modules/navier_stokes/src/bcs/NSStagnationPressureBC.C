/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes inclues
#include "NSStagnationPressureBC.h"
#include "NS.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// Full specialization of the validParams function for this object
template <>
InputParameters
validParams<NSStagnationPressureBC>()
{
  InputParameters params = validParams<NSStagnationBC>();
  params.addClassDescription("This Dirichlet condition imposes the condition p_0 = p_0_desired.");
  params.addRequiredCoupledVar(NS::pressure, "pressure");
  params.addRequiredParam<Real>("desired_stagnation_pressure", "");
  return params;
}

NSStagnationPressureBC::NSStagnationPressureBC(const InputParameters & parameters)
  : NSStagnationBC(parameters),
    _pressure(coupledValue(NS::pressure)),
    _desired_stagnation_pressure(getParam<Real>("desired_stagnation_pressure"))
{
}

Real
NSStagnationPressureBC::computeQpResidual()
{
  // p_0 = p*(1 + 0.5*(gam-1)*M^2)^(gam/(gam-1))
  const Real computed_stagnation_pressure =
      _pressure[_qp] * std::pow(1. + 0.5 * (_fp.gamma() - 1.) * _mach[_qp] * _mach[_qp],
                                _fp.gamma() / (_fp.gamma() - 1.));

  // Return the difference between the current solution's stagnation pressure
  // and the desired.  The Dirichlet condition asserts that these should be equal.
  return computed_stagnation_pressure - _desired_stagnation_pressure;
}
