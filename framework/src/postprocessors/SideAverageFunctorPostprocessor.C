//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideAverageFunctorPostprocessor.h"

registerMooseObject("MooseApp", SideAverageFunctorPostprocessor);

InputParameters
SideAverageFunctorPostprocessor::validParams()
{
  InputParameters params = SideIntegralFunctorPostprocessorTempl<false>::validParams();
  params.addClassDescription("Computes the average of a functor over a side set.");
  return params;
}

SideAverageFunctorPostprocessor::SideAverageFunctorPostprocessor(const InputParameters & parameters)
  : SideIntegralFunctorPostprocessorTempl<false>(parameters), _area(0.0)
{
}

void
SideAverageFunctorPostprocessor::initialize()
{
  SideIntegralFunctorPostprocessorTempl<false>::initialize();
  _area = 0.0;
}

void
SideAverageFunctorPostprocessor::execute()
{
  SideIntegralFunctorPostprocessorTempl<false>::execute();

  _area += this->_current_side_volume;
}

Real
SideAverageFunctorPostprocessor::getValue() const
{
  return _integral_value / _area;
}

void
SideAverageFunctorPostprocessor::finalize()
{
  SideIntegralFunctorPostprocessorTempl<false>::gatherSum(_area);
  SideIntegralFunctorPostprocessorTempl<false>::gatherSum(_integral_value);
}

void
SideAverageFunctorPostprocessor::threadJoin(const UserObject & y)
{
  SideIntegralFunctorPostprocessorTempl<false>::threadJoin(y);

  const auto & pps = static_cast<const SideAverageFunctorPostprocessor &>(y);
  _area += pps._area;
}
