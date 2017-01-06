/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowSumQuantity.h"

template<>
InputParameters validParams<PorousFlowSumQuantity>()
{
  InputParameters params = validParams<GeneralUserObject>();
  return params;
}

PorousFlowSumQuantity::PorousFlowSumQuantity(const InputParameters & parameters) :
    GeneralUserObject(parameters),
    _total(0.0)
{
}

PorousFlowSumQuantity::~PorousFlowSumQuantity()
{
}

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

