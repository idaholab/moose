//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "relaxVar.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", relaxVar);

InputParameters
relaxVar::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Relax variable.");
  params.addRequiredParam<MooseFunctorName>("var_to_relax", "Coupled turbulent kinetic energy.");
  params.addRangeCheckedParam<Real>(
      "relaxation_factor", 0, "0<=relaxation_factor<=1", "Relaxation factor.");
  return params;
}

relaxVar::relaxVar(const InputParameters & params)
  : AuxKernel(params),
    _var_to_relax(getFunctor<ADReal>("var_to_relax")),
    _relaxation_factor(getParam<Real>("relaxation_factor"))
{
}

Real
relaxVar::computeValue()
{

  return _var_to_relax(makeElemArg(_current_elem), 1).value() * _relaxation_factor +
         _var_to_relax(makeElemArg(_current_elem), 2).value() * (1.0 - _relaxation_factor);
}
