//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeAux.h"

registerMooseObject("MooseApp", VolumeAux);

InputParameters
VolumeAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Auxiliary Kernel that samples volumes.");
  return params;
}

VolumeAux::VolumeAux(const InputParameters & parameters) : AuxKernel(parameters)
{
  const auto & fe_type = mooseVariableBase()->feType();
  if (fe_type.order != CONSTANT || fe_type.family != MONOMIAL)
    paramError("variable", "Must be of type CONSTANT MONOMIAL");
}

Real
VolumeAux::computeValue()
{
  return _bnd ? _current_side_volume : _current_elem_volume;
}

void
VolumeAux::compute()
{
  _var.setDofValue(computeValue(), 0);
}
