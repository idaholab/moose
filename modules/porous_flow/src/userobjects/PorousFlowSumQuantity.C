//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowSumQuantity.h"

registerMooseObject("PorousFlowApp", PorousFlowSumQuantity);

InputParameters
PorousFlowSumQuantity::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Records total mass flowing into a borehole");
  return params;
}

PorousFlowSumQuantity::PorousFlowSumQuantity(const InputParameters & parameters)
  : GeneralUserObject(parameters), _total(0.0)
{
}

PorousFlowSumQuantity::~PorousFlowSumQuantity() {}

void
PorousFlowSumQuantity::zero()
{
  _total = 0.0;
}

void
PorousFlowSumQuantity::add(Real contrib)
{
  _total += contrib;
}

void
PorousFlowSumQuantity::initialize()
{
}

void
PorousFlowSumQuantity::execute()
{
}

void
PorousFlowSumQuantity::finalize()
{
  gatherSum(_total);
}

Real
PorousFlowSumQuantity::getValue() const
{
  return _total;
}
