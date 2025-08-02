//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUPostprocessorNeumannBC.h"

registerMooseObject("MooseApp", KokkosPostprocessorNeumannBC);

InputParameters
KokkosPostprocessorNeumannBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription(
      "Neumann boundary condition with value prescribed by a Postprocessor value.");
  params.addParam<PostprocessorName>(
      "postprocessor", 0.0, "The postprocessor to use for value of the gradient on the boundary.");
  return params;
}

KokkosPostprocessorNeumannBC::KokkosPostprocessorNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _value(getPostprocessorValue("postprocessor"))
{
}
