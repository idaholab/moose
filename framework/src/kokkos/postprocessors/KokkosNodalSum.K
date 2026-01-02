//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalSum.h"

registerKokkosPostprocessor("MooseApp", KokkosNodalSum);

InputParameters
KokkosNodalSum::validParams()
{
  InputParameters params = KokkosNodalVariablePostprocessor::validParams();
  params.set<bool>("unique_node_execute") = true;
  params.addClassDescription("Computes the sum of all of the nodal values of the specified "
                             "variable. Note: This object sets the default \"unique_node_execute\" "
                             "flag to true to avoid double counting nodes between shared blocks.");
  return params;
}

KokkosNodalSum::KokkosNodalSum(const InputParameters & parameters)
  : KokkosNodalVariablePostprocessor(parameters)
{
}

void
KokkosNodalSum::initialize()
{
  allocateReductionBuffer(1);
}

void
KokkosNodalSum::finalize()
{
  gatherSum(_reduction_buffer(0));
}

Real
KokkosNodalSum::getValue() const
{
  return _reduction_buffer(0);
}
