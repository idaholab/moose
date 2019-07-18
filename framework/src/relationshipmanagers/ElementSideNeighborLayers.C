//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementSideNeighborLayers.h"
#include "MooseMesh.h"
#include "Conversion.h"
#include "MooseApp.h"
#include "Executioner.h"
#include "FEProblemBase.h"
#include "NonlinearSystem.h"

#include "libmesh/default_coupling.h"
#include "libmesh/dof_map.h"

registerMooseObject("MooseApp", ElementSideNeighborLayers);

template <>
InputParameters
validParams<ElementSideNeighborLayers>()
{
  InputParameters params = validParams<FunctorRelationshipManager>();

  params.addRangeCheckedParam<unsigned short>(
      "layers",
      1,
      "element_side_neighbor_layers>=1 & element_side_neighbor_layers<=10",
      "The number of additional geometric elements to make available when "
      "using distributed mesh. No effect with replicated mesh.");

  return params;
}

ElementSideNeighborLayers::ElementSideNeighborLayers(const InputParameters & parameters)
  : FunctorRelationshipManager(parameters), _layers(getParam<unsigned short>("layers"))
{
}

std::string
ElementSideNeighborLayers::getInfo() const
{
  std::ostringstream oss;
  std::string layers = _layers == 1 ? "layer" : "layers";

  oss << "ElementSideNeighborLayers (" << _layers << " " << layers << ')';

  return oss.str();
}

bool
ElementSideNeighborLayers::operator==(const RelationshipManager & rhs) const
{
  const auto * rm = dynamic_cast<const ElementSideNeighborLayers *>(&rhs);
  if (!rm)
    return false;
  else
    return _layers == rm->_layers && isType(rm->_rm_type) && isSystemType(rm->_system_type);
}

void
ElementSideNeighborLayers::internalInit()
{
  auto functor = libmesh_make_unique<DefaultCoupling>();
  functor->set_n_levels(_layers);

  // Need to see if there are periodic BCs - if so we need to dig them out
  auto executioner_ptr = _app.getExecutioner();

  if (executioner_ptr)
  {
    auto & fe_problem = executioner_ptr->feProblem();
    auto & nl_sys = fe_problem.getNonlinearSystemBase();
    auto & dof_map = nl_sys.dofMap();
    auto periodic_boundaries_ptr = dof_map.get_periodic_boundaries();

    mooseAssert(periodic_boundaries_ptr, "Periodic Boundaries Pointer is nullptr");

    functor->set_mesh(&_mesh.getMesh());
    functor->set_periodic_boundaries(periodic_boundaries_ptr);
  }

  _functor = std::move(functor);
}

void
ElementSideNeighborLayers::dofmap_reinit()
{
  if (_dof_map)
    static_cast<DefaultCoupling *>(_functor.get())->set_dof_coupling(_dof_map->_dof_coupling);
}
