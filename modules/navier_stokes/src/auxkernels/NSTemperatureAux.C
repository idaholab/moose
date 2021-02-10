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

registerMooseObject("NavierStokesApp", NSTemperatureAux);

InputParameters
NSTemperatureAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("Temperature is an auxiliary value computed from the total energy "
                             "based on the FluidProperties.");
  // Mark variables as required
  params.addRequiredCoupledVar(NS::specific_volume, "specific volume");
  params.addRequiredCoupledVar(NS::specific_internal_energy, "internal energy");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  return params;
}

NSTemperatureAux::NSTemperatureAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _specific_volume(coupledValue(NS::specific_volume)),
    _specific_internal_energy(coupledValue(NS::specific_internal_energy)),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
  mooseDeprecated("The NSTemperatureAux aux kernel has been replaced by the "
                  "TemperatureAux kernel in the fluid properties module.");
}

Real
NSTemperatureAux::computeValue()
{
  return _fp.T_from_v_e(_specific_volume[_qp], _specific_internal_energy[_qp]);
}
