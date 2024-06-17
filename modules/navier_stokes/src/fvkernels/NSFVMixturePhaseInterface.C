//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVMixturePhaseInterface.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVMixturePhaseInterface);

InputParameters
NSFVMixturePhaseInterface::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Implements a phase-to-phase volumetric exchange.");
  params.addRequiredParam<MooseFunctorName>(NS::alpha,
                                            "Name of the volumetric exchange coefficient");
  params.addRequiredParam<MooseFunctorName>("phase_coupled", "The ambient temperature");
  return params;
}

NSFVMixturePhaseInterface::NSFVMixturePhaseInterface(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _alpha(getFunctor<ADReal>(NS::alpha)),
    _phase_coupled(getFunctor<ADReal>("phase_coupled"))
{
}

ADReal
NSFVMixturePhaseInterface::computeQpResidual()
{
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  return _alpha(elem_arg, state) * (_var(elem_arg, state) - _phase_coupled(elem_arg, state));
}
