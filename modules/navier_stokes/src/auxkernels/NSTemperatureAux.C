//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes inclues
#include "NSTemperatureAux.h"
#include "NS.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// MOOSE includes
#include "MooseMesh.h"

template <>
InputParameters
validParams<NSTemperatureAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addClassDescription("Temperature is an auxiliary value computed from the total energy "
                             "based on the FluidProperties.");
  // Mark variables as required
  params.addRequiredCoupledVar(NS::specific_volume, "specific volume");
  params.addRequiredCoupledVar(NS::internal_energy, "internal energy");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  return params;
}

NSTemperatureAux::NSTemperatureAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _specific_volume(coupledValue(NS::specific_volume)),
    _internal_energy(coupledValue(NS::internal_energy)),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
}

Real
NSTemperatureAux::computeValue()
{
  return _fp.temperature(_specific_volume[_qp], _internal_energy[_qp]);
}
