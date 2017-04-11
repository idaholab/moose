/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "GhostUserObject.h"
#include "MooseMesh.h"

// invalid_processor_id
#include "libmesh/dof_object.h"

template <>
InputParameters
validParams<GhostUserObject>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addParam<unsigned int>(
      "rank",
      DofObject::invalid_processor_id,
      "The rank for which the ghosted elements are recorded (Default: ALL)");

  MooseUtils::setExecuteOnFlags(params, 1, EXEC_TIMESTEP_BEGIN);
  params.addClassDescription("User object to calculate ghosted elements on a single processor or "
                             "the union across all processors.");
  return params;
}

GhostUserObject::GhostUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters), _rank(getParam<unsigned int>("rank"))
{
}

void
GhostUserObject::initialize()
{
  _ghost_data.clear();
}

void
GhostUserObject::execute()
{
  auto my_processor_id = processor_id();

  if (_rank == DofObject::invalid_processor_id || my_processor_id == _rank)
  {
    const auto & mesh = _subproblem.mesh().getMesh();

    const auto end = mesh.active_elements_end();
    for (auto el = mesh.active_elements_begin(); el != end; ++el)
    {
      const auto & elem = *el;

      if (elem->processor_id() != my_processor_id)
        _ghost_data.emplace(elem->id());
    }
  }
}

void
GhostUserObject::finalize()
{
  _communicator.set_union(_ghost_data);
}

unsigned long
GhostUserObject::getElementalValue(unsigned int element_id) const
{
  return _ghost_data.find(element_id) != _ghost_data.end();
}
