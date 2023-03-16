//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GhostingUserObject.h"
#include "NonlinearSystemBase.h"
#include "MooseMesh.h"

registerMooseObject("MooseApp", GhostingUserObject);

namespace
{
constexpr unsigned int GEOMETRIC_MAP_IDX = 0;
constexpr unsigned int ALGEBRAIC_MAP_IDX = 1;
}

InputParameters
GhostingUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addParam<std::vector<processor_id_type>>("pids", "The PID(s) to see the ghosting for");

  // No need to set or override this flag, this object is triggered by meshChanged events
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL};

  params.addClassDescription("Creates ghosting maps that can be queried by external objects.");
  return params;
}

GhostingUserObject::GhostingUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _nl(_fe_problem.getNonlinearSystemBase())
{
  _maps.resize(2); // Geometric and Algebraic maps

  if (parameters.isParamValid("pids"))
    _pids = getParam<std::vector<processor_id_type>>("pids");
  else
  {
    // If a PIDs vector is not passed, generate patterns for all processors
    _pids.resize(_app.n_processors());
    std::iota(_pids.begin(), _pids.end(), 0);
  }
}

void
GhostingUserObject::initialSetup()
{
  meshChanged();
}

void
GhostingUserObject::meshChanged()
{
  // Clear the maps before rebuilding
  for (auto & map_ref : _maps)
    for (auto pid_map_pair : map_ref)
      pid_map_pair.second.clear();

  for (auto pid : _pids)
  {
    auto begin_elem = _mesh.getMesh().active_pid_elements_begin(pid);
    const auto end_elem = _mesh.getMesh().active_pid_elements_end(pid);

    // Loop over all the geometric ghosting functors and build up a map of elements that are ghosted
    // for each PID
    libMesh::GhostingFunctor::map_type geometric_elems;
    for (auto & gf : as_range(_mesh.getMesh().ghosting_functors_begin(),
                              _mesh.getMesh().ghosting_functors_end()))
      (*gf)(begin_elem, end_elem, pid, geometric_elems);
    _maps[GEOMETRIC_MAP_IDX].emplace(pid, geometric_elems);

    // Loop over all the algebraic ghosting functors and build up a map of elements that are ghosted
    // for each PID
    libMesh::GhostingFunctor::map_type algebraic_elems;
    for (auto & gf : as_range(_nl.dofMap().algebraic_ghosting_functors_begin(),
                              _nl.dofMap().algebraic_ghosting_functors_end()))
      (*gf)(begin_elem, end_elem, pid, algebraic_elems);
    for (auto & gf :
         as_range(_nl.dofMap().coupling_functors_begin(), _nl.dofMap().coupling_functors_end()))
      (*gf)(begin_elem, end_elem, pid, algebraic_elems);
    _maps[ALGEBRAIC_MAP_IDX].emplace(pid, algebraic_elems);
  }
}

Real
GhostingUserObject::getElementalValue(const Elem * elem,
                                      Moose::RelationshipManagerType rm_type,
                                      processor_id_type pid) const
{
  unsigned int map_idx = (rm_type == Moose::RelationshipManagerType::GEOMETRIC ? GEOMETRIC_MAP_IDX
                                                                               : ALGEBRAIC_MAP_IDX);

  auto map_it = _maps[map_idx].find(pid);
  if (map_it == _maps[map_idx].end())
    mooseError("No entry in the ghosting map for processor ID: ", pid);

  auto map_ref = map_it->second;
  if (map_ref.find(elem) != map_ref.end())
    return 1.;
  else
    return 0.;
}
