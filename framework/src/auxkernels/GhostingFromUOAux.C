//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GhostingFromUOAux.h"
#include "GhostingUserObject.h"

registerMooseObject("MooseApp", GhostingFromUOAux);
registerMooseObjectRenamed("MooseApp", GhostingAux, "04/01/2025 00:00", GhostingFromUOAux);

InputParameters
GhostingFromUOAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredParam<processor_id_type>("pid", "The PID to see the ghosting for");

  MooseEnum functor_type("geometric algebraic", "geometric");
  params.addParam<MooseEnum>("functor_type", functor_type, "The type of ghosting functor to use");

  params.addParam<bool>("include_local_elements",
                        false,
                        "Whether or not to include local elements as part of the ghosting set");

  params.addRequiredParam<UserObjectName>(
      "ghost_uo", "The GhostUserObject from which to obtain ghosting information from.");

  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};

  params.addClassDescription("Colors the elements ghosted to the chosen PID.");
  return params;
}

GhostingFromUOAux::GhostingFromUOAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _pid(getParam<processor_id_type>("pid")),
    _rm_type(getParam<MooseEnum>("functor_type") == "geometric"
                 ? Moose::RelationshipManagerType::GEOMETRIC
                 : Moose::RelationshipManagerType::ALGEBRAIC),
    _include_local(getParam<bool>("include_local_elements")),
    _value(0.),
    _ghost_uo(getUserObject<GhostingUserObject>("ghost_uo"))
{
  if (isNodal())
    mooseError("GhostingFromUOAux only works on elemental fields.");
}

void
GhostingFromUOAux::precalculateValue()
{
  if (_current_elem->processor_id() == _pid && _include_local)
    _value = 1;
  else if (_current_elem->processor_id() != _pid)
    _value = _ghost_uo.getElementalValue(_current_elem, _rm_type, _pid);
  else
    _value = 0.;
}

Real
GhostingFromUOAux::computeValue()
{
  return _value;
}
