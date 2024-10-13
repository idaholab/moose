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
#include "libmesh/point_neighbor_coupling.h"
#include "libmesh/dof_map.h"

using namespace libMesh;

registerMooseObject("MooseApp", ElementSideNeighborLayers);

InputParameters
ElementSideNeighborLayers::validParams()
{
  InputParameters params = FunctorRelationshipManager::validParams();

  params.addRangeCheckedParam<unsigned short>(
      "layers",
      1,
      "element_side_neighbor_layers>=1 & element_side_neighbor_layers<=10",
      "The number of additional geometric elements to make available when "
      "using distributed mesh. No effect with replicated mesh.");
  params.addParam<bool>("use_point_neighbors",
                        false,
                        "Whether to use point neighbors, which introduces additional ghosting to "
                        "that used for simple face neighbors.");

  return params;
}

ElementSideNeighborLayers::ElementSideNeighborLayers(const InputParameters & parameters)
  : FunctorRelationshipManager(parameters),
    _layers(getParam<unsigned short>("layers")),
    _use_point_neighbors(getParam<bool>("use_point_neighbors"))
{
}

ElementSideNeighborLayers::ElementSideNeighborLayers(const ElementSideNeighborLayers & other)
  : FunctorRelationshipManager(other),
    _layers(other._layers),
    _use_point_neighbors(other._use_point_neighbors)
{
}

std::unique_ptr<GhostingFunctor>
ElementSideNeighborLayers::clone() const
{
  return _app.getFactory().copyConstruct(*this);
}

std::string
ElementSideNeighborLayers::getInfo() const
{
  std::ostringstream oss;
  std::string layers = _layers == 1 ? "layer" : "layers";

  oss << "ElementSideNeighborLayers (" << _layers << " " << layers << ')';

  return oss.str();
}

// the LHS ("this" object) in MooseApp::addRelationshipManager is the existing RelationshipManager
// object to which we are comparing the rhs to determine whether it should get added
bool
ElementSideNeighborLayers::operator>=(const RelationshipManager & rhs) const
{
  const auto * rm = dynamic_cast<const ElementSideNeighborLayers *>(&rhs);
  if (!rm)
    return false;
  else
    // We use a >= comparison instead of == for _layers because if we already have more ghosting
    // than the new RM provides, then that's an indication that we should *not* add the new one
    return (_layers >= rm->_layers) && (_use_point_neighbors == rm->_use_point_neighbors) &&
           baseGreaterEqual(*rm);
}

template <typename Functor>
void
ElementSideNeighborLayers::initFunctor(Functor & functor)
{
  functor.set_n_levels(_layers);

  if (_dof_map)
  {
    // Need to see if there are periodic BCs - if so we need to dig them out
    auto periodic_boundaries_ptr = _dof_map->get_periodic_boundaries();

    mooseAssert(periodic_boundaries_ptr, "Periodic Boundaries Pointer is nullptr");

    functor.set_periodic_boundaries(periodic_boundaries_ptr);
    functor.set_dof_coupling(_dof_map->_dof_coupling);
  }
}

void
ElementSideNeighborLayers::internalInitWithMesh(const MeshBase &)
{
  if (_use_point_neighbors)
  {
    auto functor = std::make_unique<PointNeighborCoupling>();
    initFunctor(*functor);
    _functor = std::move(functor);
  }
  else
  {
    auto functor = std::make_unique<DefaultCoupling>();
    initFunctor(*functor);
    _functor = std::move(functor);
  }
}

void
ElementSideNeighborLayers::dofmap_reinit()
{
  if (_dof_map)
  {
    if (_use_point_neighbors)
      static_cast<PointNeighborCoupling *>(_functor.get())
          ->set_dof_coupling(_dof_map->_dof_coupling);
    else
      static_cast<DefaultCoupling *>(_functor.get())->set_dof_coupling(_dof_map->_dof_coupling);
  }
}
