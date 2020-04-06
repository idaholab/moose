//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElemExtraIDAux.h"

registerMooseObject("MooseApp", ElemExtraIDAux);

defineLegacyParams(ElemExtraIDAux);

InputParameters
ElemExtraIDAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<std::vector<ExtraElementIDName>>("extra_id_name",
                                                           "The extra ID name in the mesh");
  params.addClassDescription("Puts element extra IDs into an aux variable.");
  return params;
}

ElemExtraIDAux::ElemExtraIDAux(const InputParameters & parameters)
  : AuxKernel(parameters), _id(getElementID("extra_id_name"))
{
}

Real
ElemExtraIDAux::computeValue()
{
  return _id;
}
