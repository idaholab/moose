//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVSideSetHeatTransferKernel.h"

registerMooseObject("HeatTransferApp", FVSideSetHeatTransferKernel);

InputParameters
FVSideSetHeatTransferKernel::validParams()
{
  InputParameters params = FVInterfaceKernel::validParams();
  params.addClassDescription("Models heat transfer across an internal sideset for the finite "
                             "volume method using an interface conductance.");

  params.addRequiredParam<MooseFunctorName>(
      "conductance",
      "Thermal conductance across the interface. This is equivalent to k / thickness or "
      "1 / thermal_resistance. The conductance functor is evaluated on the variable1/subdomain1 "
      "side of the interface.");

  return params;
}

FVSideSetHeatTransferKernel::FVSideSetHeatTransferKernel(const InputParameters & parameters)
  : FVInterfaceKernel(parameters), _conductance(getFunctor<ADReal>("conductance"))
{
}

ADReal
FVSideSetHeatTransferKernel::computeQpResidual()
{
  const auto state = determineState();

  const auto T1 = var1().getElemValue(&elem1(), state);
  const auto T2 = var2().getElemValue(&elem2(), state);
  const auto conductance = _conductance(faceArg1(), state);

  return conductance * (T1 - T2);
}
