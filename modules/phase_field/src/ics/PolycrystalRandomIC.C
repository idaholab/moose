//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalRandomIC.h"
#include "MooseRandom.h"

template <>
InputParameters
validParams<PolycrystalRandomIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addClassDescription("Random initial condition for a polycrystalline material");
  params.addRequiredParam<unsigned int>("op_num", "Number of order parameters");
  params.addRequiredParam<unsigned int>("op_index", "The index for the current order parameter");
  params.addRequiredParam<unsigned int>(
      "typ", "Type of random grain structure"); // TODO: this should be called "type"!
  return params;
}

PolycrystalRandomIC::PolycrystalRandomIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _op_num(getParam<unsigned int>("op_num")),
    _op_index(getParam<unsigned int>("op_index")),
    _typ(getParam<unsigned int>("typ"))
{
}

Real
PolycrystalRandomIC::value(const Point & p)
{
  Point cur_pos = p;
  Real val = MooseRandom::rand();

  switch (_typ)
  {
    case 0: // Continuously random
      return val;

    case 1: // Discretely random
    {
      unsigned int rndind = _op_num * val;

      if (rndind == _op_index)
        return 1.0;
      else
        return 0.0;
    }
  }

  mooseError("Bad type passed in PolycrystalRandomIC");
}
