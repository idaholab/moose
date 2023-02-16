//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVLiquidFraction.h"
#include "MooseMesh.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVLiquidFraction);

InputParameters
NSFVLiquidFraction::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes liquid fraction $f_l$ given the temperature.");
  params.addRequiredParam<MooseFunctorName>(NS::temperature, "The temperature.");
  params.addRequiredParam<MooseFunctorName>("T_solidus", "The solidus temperature.");
  params.addRequiredParam<MooseFunctorName>("T_liquidus", "The liquidus temperature.");
  return params;
}

NSFVLiquidFraction::NSFVLiquidFraction(const InputParameters & parameters)
  : AuxKernel(parameters),
    _T(getFunctor<ADReal>(NS::temperature)),
    _T_solidus(getFunctor<ADReal>("T_solidus")),
    _T_liquidus(getFunctor<ADReal>("T_liquidus"))
{
}

Real
NSFVLiquidFraction::computeValue()
{
  using namespace MetaPhysicL;

  const auto elem_arg = makeElemArg(_current_elem);

  if (raw_value(_T_liquidus(elem_arg)) < raw_value(_T_solidus(elem_arg)))
    mooseError("The specified liquidus temperature is smaller than the solidus temperature.");

  Real fl = raw_value((_T(elem_arg) - _T_solidus(elem_arg)) /
                      (_T_liquidus(elem_arg) - _T_solidus(elem_arg)));

  fl = (fl > 1.0) ? 1.0 : fl;
  fl = (fl < 0.0) ? 0.0 : fl;

  return fl;
}
