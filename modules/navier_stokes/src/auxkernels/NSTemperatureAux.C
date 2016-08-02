/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NSTemperatureAux.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// MOOSE includes
#include "MooseMesh.h"

template<>
InputParameters validParams<NSTemperatureAux>()
{
  InputParameters params = validParams<AuxKernel>();

  // Mark variables as required
  params.addRequiredCoupledVar("specific_volume", "");
  params.addRequiredCoupledVar("internal_energy", "");
  params.addRequiredParam<UserObjectName>("fluid_properties", "The name of the user object for fluid properties");

  return params;
}

NSTemperatureAux::NSTemperatureAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _specific_volume(coupledValue("specific_volume")),
    _internal_energy(coupledValue("internal_energy")),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
}

Real
NSTemperatureAux::computeValue()
{
  return _fp.temperature(_specific_volume[_qp], _internal_energy[_qp]);
}
