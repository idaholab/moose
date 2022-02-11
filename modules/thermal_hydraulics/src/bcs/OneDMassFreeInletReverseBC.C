//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDMassFreeInletReverseBC.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", OneDMassFreeInletReverseBC);

InputParameters
OneDMassFreeInletReverseBC::validParams()
{
  InputParameters params = OneDMassFreeBC::validParams();
  params.addRequiredParam<bool>("reversible",
                                "true if the boundary condition is reversible, otherwise false.");
  return params;
}

OneDMassFreeInletReverseBC::OneDMassFreeInletReverseBC(const InputParameters & parameters)
  : OneDMassFreeBC(parameters),
    _reversible(getParam<bool>("reversible")),
    _arhouA_old(coupledValueOld("arhouA"))
{
}

bool
OneDMassFreeInletReverseBC::shouldApply()
{
  return !_reversible || THM::isOutlet(_arhouA_old[0], _normal);
}
