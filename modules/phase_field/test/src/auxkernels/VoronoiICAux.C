//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VoronoiICAux.h"

registerMooseObject("PhaseFieldTestApp", VoronoiICAux);

InputParameters
VoronoiICAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<UserObjectName>("polycrystal_ic_uo", "Optional: Polycrystal IC object");
  return params;
}

VoronoiICAux::VoronoiICAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _poly_ic_uo(getUserObject<PolycrystalUserObjectBase>("polycrystal_ic_uo"))
{
  if (isNodal())
    mooseError("This AuxKernel only supports Elemental fields");
}

Real
VoronoiICAux::computeValue()
{
  _poly_ic_uo.getGrainsBasedOnElem(*_current_elem, _grain_ids);

  if (_grain_ids.empty())
    return 0;
  else
    return _grain_ids[0];
}
