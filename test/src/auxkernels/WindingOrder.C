//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WindingOrder.h"

registerMooseObject("MooseTestApp", WindingOrder);

InputParameters
WindingOrder::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes the winding order of the current 2D element.");
  return params;
}

WindingOrder::WindingOrder(const InputParameters & parameters) : AuxKernel(parameters)
{
  if (isNodal())
    mooseError(" WindingOrder must be for an elemental variable");
  if (_mesh.dimension() != 2)
    mooseError(" WindingOrder is only defined for 2D elements");
}

Real
WindingOrder::computeValue()
{
  const unsigned int n = _current_elem->n_nodes();
  Real v = 0.0;
  for (unsigned int i1 = 0; i1 < n; ++i1)
  {
    const auto i2 = (i1 + 1) % n;
    const auto & n1 = _current_elem->node_ref(i1);
    const auto & n2 = _current_elem->node_ref(i2);
    v += (n2(0) - n1(0)) * (n2(1) + n1(1));
  }
  return v;
}
