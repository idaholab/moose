//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVEnergyTimeDerivative.h"
#include "INSFVEnergyVariable.h"

#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVEnergyTimeDerivative);

InputParameters
INSFVEnergyTimeDerivative::validParams()
{
  InputParameters params = FVFunctorTimeKernel::validParams();
  params.addClassDescription(
      "Adds the time derivative term to the incompressible Navier-Stokes energy equation.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::cp, "Specific heat capacity");
  return params;
}

INSFVEnergyTimeDerivative::INSFVEnergyTimeDerivative(const InputParameters & params)
  : FVFunctorTimeKernel(params),
    _rho(getFunctor<ADReal>(getParam<MooseFunctorName>(NS::density))),
    _cp(getFunctor<ADReal>(getParam<MooseFunctorName>(NS::cp)))
{
  if (!dynamic_cast<INSFVEnergyVariable *>(&_var))
    paramError("variable", "The supplied variable should be of INSFVEnergyVariable type.");
}

ADReal
INSFVEnergyTimeDerivative::computeQpResidual()
{
  const auto & elem_arg = makeElemArg(_current_elem);
  return _rho(elem_arg, determineState()) * _cp(elem_arg, determineState()) *
         _var.dot(elem_arg, determineState());
}
