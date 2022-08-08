//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "BoundaryElemIntegrityCheckThread.h"
#include "AuxiliarySystem.h"
#include "NonlinearSystemBase.h"
#include "FEProblemBase.h"
#include "SideUserObject.h"
#include "MooseMesh.h"
#include "IntegratedBCBase.h"
#include "MooseObjectTagWarehouse.h"

#include "libmesh/threads.h"
#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"

#include <vector>

BoundaryElemIntegrityCheckThread::BoundaryElemIntegrityCheckThread(
    FEProblemBase & fe_problem, const TheWarehouse::Query & query)
  : _fe_problem(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _elem_aux(_aux_sys.elemAuxWarehouse()),
    _elem_vec_aux(_aux_sys.elemVectorAuxWarehouse()),
    _elem_array_aux(_aux_sys.elemArrayAuxWarehouse()),
    _integrated_bcs(fe_problem.getNonlinearSystemBase().getIntegratedBCWarehouse()),
    _query(query)
{
}

// Splitting Constructor
BoundaryElemIntegrityCheckThread::BoundaryElemIntegrityCheckThread(
    BoundaryElemIntegrityCheckThread & x, Threads::split)
  : _fe_problem(x._fe_problem),
    _aux_sys(x._aux_sys),
    _elem_aux(x._elem_aux),
    _elem_vec_aux(x._elem_vec_aux),
    _elem_array_aux(x._elem_array_aux),
    _integrated_bcs(x._integrated_bcs),
    _query(x._query)
{
}

void
BoundaryElemIntegrityCheckThread::operator()(const ConstBndElemRange & range)
{
  ParallelUniqueId puid;
  const auto tid = puid.id;

  for (const auto & belem : range)
  {
    const Elem * elem = belem->_elem;
    const auto boundary_id = belem->_bnd_id;
    const auto side = belem->_side;

    // We can distribute work just as the actual execution code will
    if (elem->processor_id() != _fe_problem.processor_id())
      return;

    auto & mesh = _fe_problem.mesh();

    const auto & bnd_name = mesh.getBoundaryName(boundary_id);

    // uo check
    std::vector<SideUserObject *> objs;
    _query.clone()
        .condition<AttribThread>(tid)
        .condition<AttribInterfaces>(Interfaces::SideUserObject)
        .condition<AttribBoundaries>(boundary_id, true)
        .queryInto(objs);
    for (const auto & uo : objs)
      if (uo->checkVariableBoundaryIntegrity())
      {
        auto leftover_vars = uo->checkAllVariables(*elem);
        if (!leftover_vars.empty())
        {
          const auto neighbor = elem->neighbor_ptr(side);
          const bool upwind_elem = !neighbor || elem->id() < neighbor->id();
          const Elem * lower_d_elem =
              upwind_elem ? mesh.getLowerDElem(elem, side)
                          : mesh.getLowerDElem(neighbor, neighbor->which_neighbor_am_i(elem));
          if (lower_d_elem)
            leftover_vars = uo->checkVariables(*lower_d_elem, leftover_vars);
        }
        boundaryIntegrityCheckError(*uo, leftover_vars, bnd_name);
      }

    auto check = [elem, boundary_id, &bnd_name, tid, &mesh, side](const auto & warehouse)
    {
      if (!warehouse.hasBoundaryObjects(boundary_id, tid))
        return;

      const auto & bnd_objects = warehouse.getBoundaryObjects(boundary_id, tid);
      for (const auto & bnd_object : bnd_objects)
        // Skip if this object uses geometric search because coupled variables may be defined on
        // paired boundaries instead of the boundary this elem is on
        if (!bnd_object->requiresGeometricSearch() && bnd_object->checkVariableBoundaryIntegrity())
        {
          // First check the higher-dimensional element
          auto leftover_vars = bnd_object->checkAllVariables(*elem);
          if (!leftover_vars.empty())
          {
            const auto neighbor = elem->neighbor_ptr(side);
            const bool upwind_elem = !neighbor || elem->id() < neighbor->id();
            const Elem * lower_d_elem =
                upwind_elem ? mesh.getLowerDElem(elem, side)
                            : mesh.getLowerDElem(neighbor, neighbor->which_neighbor_am_i(elem));
            if (lower_d_elem)
              leftover_vars = bnd_object->checkVariables(*lower_d_elem, leftover_vars);
          }

          boundaryIntegrityCheckError(*bnd_object, leftover_vars, bnd_name);
        }
    };

    check(_elem_aux);
    check(_elem_vec_aux);
    check(_elem_array_aux);
    check(_integrated_bcs);
  }
}

void
BoundaryElemIntegrityCheckThread::join(const BoundaryElemIntegrityCheckThread & /*y*/)
{
}

void
boundaryIntegrityCheckError(const MooseObject & object,
                            const std::set<MooseVariableFieldBase *> & variables,
                            const BoundaryName & boundary_name)
{
  if (variables.empty())
    return;

  std::vector<std::string> names;
  names.reserve(variables.size());
  for (const auto * const var : variables)
    names.push_back(var->name());

  mooseError("'",
             object.name(),
             "' of type '",
             object.type(),
             "' depends on variable(s) '",
             MooseUtils::join(names, ","),
             "'. However, that variable does not appear to be defined on (all of) boundary '",
             boundary_name,
             "'.");
}
