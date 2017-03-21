/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
