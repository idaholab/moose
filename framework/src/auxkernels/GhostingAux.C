//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GhostingAux.h"

registerMooseObject("MooseApp", GhostingAux);

template <>
InputParameters
validParams<GhostingAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addClassDescription("Colors the elements ghosted to the chosen PID.");

  params.addRequiredParam<processor_id_type>("pid", "The PID to see the ghosting for");

  MooseEnum functor_type("geometric algebraic", "geometric");

  params.addParam<MooseEnum>("functor_type", functor_type, "The type of ghosting functor to use");

  return params;
}

GhostingAux::GhostingAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _pid(getParam<processor_id_type>("pid")),
    _functor_type(getParam<MooseEnum>("functor_type"))
{
  if (isNodal())
    mooseError("GhostingAux only works on elemental fields.");

  // Can only work on Replicated meshes
  _mesh.errorIfDistributedMesh("GhostingAux");
}

void
GhostingAux::initialize()
{
  _ghosted_elems.clear();

  if (_functor_type == 0) // Geometric
  {
    auto current_func = _mesh.getMesh().ghosting_functors_begin();
    const auto end_func = _mesh.getMesh().ghosting_functors_end();

    const auto begin_elem = _mesh.getMesh().active_pid_elements_begin(_pid);
    const auto end_elem = _mesh.getMesh().active_pid_elements_end(_pid);

    for (; current_func != end_func; ++current_func)
      (*(*current_func))(begin_elem, end_elem, _pid, _ghosted_elems);
  }
  else // Algebraic
  {
    std::cout << "Doing algebraic for pid: " << _pid << std::endl;

    auto current_func = _nl_sys.dofMap().algebraic_ghosting_functors_begin();
    const auto end_func = _nl_sys.dofMap().algebraic_ghosting_functors_end();

    const auto begin_elem = _mesh.getMesh().active_pid_elements_begin(_pid);
    const auto end_elem = _mesh.getMesh().active_pid_elements_end(_pid);

    for (; current_func != end_func; ++current_func)
    {

      (*(*current_func))(begin_elem, end_elem, _pid, _ghosted_elems);
    }
  }
}

Real
GhostingAux::computeValue()
{
  if (_current_elem->processor_id() != _pid &&
      _ghosted_elems.find(_current_elem) != _ghosted_elems.end())
    return 1.;
  else
    return 0.;
}
