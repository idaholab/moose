//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExtraElementIDAux.h"

registerMooseObject("MooseApp", ExtraElementIDAux);
registerMooseObjectRenamed("MooseApp", ElemExtraIDAux, "01/30/2022 24:00", ExtraElementIDAux);

InputParameters
ExtraElementIDAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<std::vector<ExtraElementIDName>>("extra_id_name",
                                                           "The extra ID name in the mesh");
  params.addClassDescription("Puts element extra IDs into an aux variable.");
  return params;
}

ExtraElementIDAux::ExtraElementIDAux(const InputParameters & parameters)
  : AuxKernel(parameters), _id(getElementID("extra_id_name"))
{
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
}

Real
ExtraElementIDAux::computeValue()
{
  return (_id == DofObject::invalid_id) ? -1.0 : _id;
}
