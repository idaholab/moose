//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVMomentumPressure.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVMomentumPressure);

InputParameters
INSFVMomentumPressure::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();
  params.addClassDescription(
      "Introduces the coupled pressure term into the Navier-Stokes momentum equation.");
  params.addRequiredParam<MooseFunctorName>(NS::pressure, "The pressure");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addParam<bool>(
      "correct_skewness", false, "Whether to correct for mesh skewness in face calculations.");
  return params;
}

INSFVMomentumPressure::INSFVMomentumPressure(const InputParameters & params)
  : FVElementalKernel(params),
    INSFVMomentumResidualObject(*this),
    _p(getFunctor<ADReal>(NS::pressure)),
    _index(getParam<MooseEnum>("momentum_component")),
    _correct_skewness(getParam<bool>("correct_skewness"))
{
}

ADReal
INSFVMomentumPressure::computeQpResidual()
{
  return _p.gradient(Moose::ElemArg{_current_elem, _correct_skewness}, determineState())(_index);
}
