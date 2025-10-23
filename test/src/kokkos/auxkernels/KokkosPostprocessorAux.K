//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosPostprocessorAux.h"

registerKokkosAuxKernel("MooseTestApp", KokkosPostprocessorAux);

InputParameters
KokkosPostprocessorAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<PostprocessorName>("pp", "The Postprocessor to use as the value");
  return params;
}

KokkosPostprocessorAux::KokkosPostprocessorAux(const InputParameters & parameters)
  : AuxKernel(parameters), _pp_val(getPostprocessorValue("pp"))
{
}
