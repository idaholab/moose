//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalVoidVolumeAux.h"

registerMooseObject("GeochemistryApp", NodalVoidVolumeAux);

InputParameters
NodalVoidVolumeAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Extracts information from the NodalVoidVolume UserObject and records it in the AuxVariable");
  params.addRequiredParam<UserObjectName>("nodal_void_volume_uo",
                                          "The name of the NodalVoidVolume UserObject.");
  return params;
}

NodalVoidVolumeAux::NodalVoidVolumeAux(const InputParameters & parameters)
  : AuxKernel(parameters), _nvv(getUserObject<NodalVoidVolume>("nodal_void_volume_uo"))
{
  if (!_var.isNodal())
    mooseError("NodalVoidVolumeAux: variable must be nodal (eg, linear Lagrange)");
}

Real
NodalVoidVolumeAux::computeValue()
{
  return _nvv.getNodalVoidVolume(_current_node);
}
