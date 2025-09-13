//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosBodyForce.h"

registerKokkosResidualObject("MooseApp", KokkosBodyForce);

InputParameters
KokkosBodyForce::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<Real>("value", 1.0, "Coefficient to multiply by the body force term");
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
  params.declareControllable("value");
  params.addClassDescription("Demonstrates the two ways that scalar values can be introduced "
                             "into kernels, e.g. (controllable) constants and postprocessors. "
                             "Implements the weak form $(\\psi_i, -f)$.");
  return params;
}

KokkosBodyForce::KokkosBodyForce(const InputParameters & parameters)
  : Kernel(parameters),
    _scale(getParam<Real>("value")),
    _postprocessor(getPostprocessorValue("postprocessor"))
{
}
