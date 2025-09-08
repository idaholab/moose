//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementHierarchyAux.h"

registerMooseObject("MooseApp", ElementHierarchyAux);

InputParameters
ElementHierarchyAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("stores the element hierarchy in a aux variable");

  return params;
}

ElementHierarchyAux::ElementHierarchyAux(const InputParameters & parameters) : AuxKernel(parameters)
{
}

Real
ElementHierarchyAux::computeValue()
{
  return static_cast<Real>(_current_elem->level());
}
