//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSLiquidFractionAux.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSLiquidFractionAux);

InputParameters
NSLiquidFractionAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes liquid fraction $f_l$ given the temperature.");
  params.addRequiredParam<MooseFunctorName>(NS::temperature, "The temperature.");
  params.addRequiredParam<MooseFunctorName>("T_solidus", "The solidus temperature.");
  params.addRequiredParam<MooseFunctorName>("T_liquidus", "The liquidus temperature.");
  return params;
}

NSLiquidFractionAux::NSLiquidFractionAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _T(getFunctor<ADReal>(NS::temperature)),
    _T_solidus(getFunctor<ADReal>("T_solidus")),
    _T_liquidus(getFunctor<ADReal>("T_liquidus"))
{
}

Real
NSLiquidFractionAux::computeValue()
{
  using namespace MetaPhysicL;

  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();

  if (raw_value(_T_liquidus(elem_arg, state)) < raw_value(_T_solidus(elem_arg, state)))
    mooseError("The specified liquidus temperature is smaller than the solidus temperature.");

  Real fl = raw_value((_T(elem_arg, state) - _T_solidus(elem_arg, state)) /
                      (_T_liquidus(elem_arg, state) - _T_solidus(elem_arg, state)));

  fl = (fl > 1.0) ? 1.0 : fl;
  fl = (fl < 0.0) ? 0.0 : fl;

  return fl;
}
