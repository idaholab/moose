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
  params.addRequiredCoupledVar(NS::pressure, "The pressure");
  params.addDeprecatedCoupledVar("p", NS::pressure, "1/1/2022");
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
    _p(coupledValue(NS::pressure)),
    _eps(getFunctor<ADReal>(NS::porosity)),
    _index(getParam<MooseEnum>("momentum_component"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("PINSFV is not supported by local AD indexing. In order to use PINSFV, please run "
             "the configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

  if (!dynamic_cast<PINSFVSuperficialVelocityVariable *>(&_var))
    mooseError(
        "PINSFVMomentumPressurePorosityGradient may only be used with a superficial velocity "
        "variable, of variable type PINSFVSuperficialVelocityVariable.");
}

ADReal
PINSFVMomentumPressurePorosityGradient::computeQpResidual()
{
  return -_p[_qp] * _eps.gradient(makeElemArg(_current_elem))(_index);
}
