//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumPressurePorosityGradient.h"
#include "PINSFVSuperficialVelocityVariable.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumPressurePorosityGradient);

InputParameters
PINSFVMomentumPressurePorosityGradient::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Introduces the coupled pressure times porosity gradient term "
                             "into the Navier-Stokes porous media momentum equation.");
  params.addRequiredParam<MooseFunctorName>(NS::pressure, "The pressure");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity auxiliary variable");

  return params;
}

PINSFVMomentumPressurePorosityGradient::PINSFVMomentumPressurePorosityGradient(
    const InputParameters & params)
  : FVElementalKernel(params),
    _p(getFunctor<ADReal>(NS::pressure)),
    _eps(getFunctor<ADReal>(NS::porosity)),
    _index(getParam<MooseEnum>("momentum_component"))
{
  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError(
        "PINSFVMomentumPressurePorosityGradient may only be used with a superficial velocity "
        "variable, of variable type PINSFVSuperficialVelocityVariable.");
}

ADReal
PINSFVMomentumPressurePorosityGradient::computeQpResidual()
{
  const auto & elem = makeElemArg(_current_elem);
  const auto state = determineState();
  return -_p(elem, state) * _eps.gradient(elem, state)(_index);
}
