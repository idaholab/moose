//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  if (mooseVariableBase()->feType() != libMesh::FEType(CONSTANT, MONOMIAL))
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
