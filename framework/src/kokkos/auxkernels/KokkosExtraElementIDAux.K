//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosExtraElementIDAux.h"

registerKokkosAuxKernel("MooseApp", KokkosExtraElementIDAux);

InputParameters
KokkosExtraElementIDAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<std::vector<ExtraElementIDName>>("extra_id_name",
                                                           "The extra ID name in the mesh");
  params.addClassDescription("Puts element extra IDs into an aux variable.");
  return params;
}

KokkosExtraElementIDAux::KokkosExtraElementIDAux(const InputParameters & parameters)
  : AuxKernel(parameters), _index(getElementIDIndex("extra_id_name"))
{
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
}
