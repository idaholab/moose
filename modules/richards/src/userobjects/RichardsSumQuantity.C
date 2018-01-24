//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsSumQuantity.h"

template <>
InputParameters
validParams<RichardsSumQuantity>()
{
  InputParameters params = validParams<GeneralUserObject>();

  return params;
}

RichardsSumQuantity::RichardsSumQuantity(const InputParameters & parameters)
  : GeneralUserObject(parameters), _total(0)
{
}

RichardsSumQuantity::~RichardsSumQuantity() {}

void
RichardsSumQuantity::zero()
{
  _total = 0;
}

void
RichardsSumQuantity::add(Real contrib)
{
  _total += contrib;
}

void
RichardsSumQuantity::initialize()
{
}

void
RichardsSumQuantity::execute()
{
}

void
RichardsSumQuantity::finalize()
{
  gatherSum(_total);
}

Real
RichardsSumQuantity::getValue() const
{
  return _total;
}
