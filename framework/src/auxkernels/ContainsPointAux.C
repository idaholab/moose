//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContainsPointAux.h"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", ContainsPointAux);

InputParameters
ContainsPointAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<Point>("point", "The point we're checking containment of");
  params.addClassDescription("Computes a binary field where the field is 1 in the elements that "
                             "contain the point  and 0 everywhere else");
  return params;
}

ContainsPointAux::ContainsPointAux(const InputParameters & parameters)
  : AuxKernel(parameters), _point(getParam<Point>("point"))
{
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
}

Real
ContainsPointAux::computeValue()
{
  return _current_elem->contains_point(_point);
}
