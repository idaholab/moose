//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeometricSearchData.h"
// Moose includes
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"
#include "ElementPairLocator.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "Assembly.h"

#include "libmesh/elem.h"
#include "libmesh/node.h"

GeometricSearchData::GeometricSearchData(SubProblem & subproblem, MooseMesh & mesh)
  : _subproblem(subproblem), _mesh(mesh), _first(true)
{
}

GeometricSearchData::~GeometricSearchData()
{
  for (auto & it : _penetration_locators)
    delete it.second;

  for (auto & it : _nearest_node_locators)
    delete it.second;
}

void
GeometricSearchData::update(GeometricSearchType type)
{
  if (type == ALL || type == QUADRATURE || type == NEAREST_NODE)
  {
    if (_first) // Only do this once
    {
      _first = false;

      for (const auto & it : _secondary_to_qsecondary)
        generateQuadratureNodes(it.first, it.second);

      // reinit on displaced mesh before update
      for (const auto & epl_it : _element_pair_locators)
      {
        ElementPairLocator & epl = *(epl_it.second);
        epl.reinit();
      }
    }

    // Update the position of quadrature nodes first
    for (const auto & qbnd : _quadrature_boundaries)
      updateQuadratureNodes(qbnd);
  }

  if (type == ALL || type == NEAREST_NODE)
  {
    for (const auto & nnl_it : _nearest_node_locators)
    {
      NearestNodeLocator * nnl = nnl_it.second;
      nnl->findNodes();
    }
  }

  if (type == ALL || type == PENETRATION)
  {
    for (const auto & pl_it : _penetration_locators)
    {
      PenetrationLocator * pl = pl_it.second;
      pl->detectPenetration();
    }
  }

  if (type == ALL || type == PENETRATION)
  {
    for (auto & elem_pair_locator_pair : _element_pair_locators)
    {
      ElementPairLocator & epl = (*elem_pair_locator_pair.second);
      epl.update();
    }
  }
}

void
GeometricSearchData::reinit()
{
  _mesh.clearQuadratureNodes();
  // Update the position of quadrature nodes first
  for (const auto & qbnd : _quadrature_boundaries)
    reinitQuadratureNodes(qbnd);

  for (const auto & nnl_it : _nearest_node_locators)
  {
    NearestNodeLocator * nnl = nnl_it.second;
    nnl->reinit();
  }

  for (const auto & pl_it : _penetration_locators)
  {
    PenetrationLocator * pl = pl_it.second;
    pl->reinit();
  }

  for (const auto & epl_it : _element_pair_locators)
  {
    ElementPairLocator & epl = *(epl_it.second);
    epl.reinit();
  }
}

void
GeometricSearchData::clearNearestNodeLocators()
{
  for (const auto & nnl_it : _nearest_node_locators)
  {
    NearestNodeLocator * nnl = nnl_it.second;
    nnl->reinit();
  }
}

Real
GeometricSearchData::maxPatchPercentage()
{
  Real max = 0.0;

  for (const auto & nnl_it : _nearest_node_locators)
  {
    NearestNodeLocator * nnl = nnl_it.second;

    if (nnl->_max_patch_percentage > max)
      max = nnl->_max_patch_percentage;
  }

  return max;
}

PenetrationLocator &
GeometricSearchData::getPenetrationLocator(const BoundaryName & primary,
                                           const BoundaryName & secondary,
                                           Order order)
{
  unsigned int primary_id = _mesh.getBoundaryID(primary);
  unsigned int secondary_id = _mesh.getBoundaryID(secondary);

  _subproblem.addGhostedBoundary(primary_id);
  _subproblem.addGhostedBoundary(secondary_id);

  PenetrationLocator * pl =
      _penetration_locators[std::pair<unsigned int, unsigned int>(primary_id, secondary_id)];

  if (!pl)
  {
    pl = new PenetrationLocator(_subproblem,
                                *this,
                                _mesh,
                                primary_id,
                                secondary_id,
                                order,
                                getNearestNodeLocator(primary_id, secondary_id));
    _penetration_locators[std::pair<unsigned int, unsigned int>(primary_id, secondary_id)] = pl;
  }

  return *pl;
}

PenetrationLocator &
GeometricSearchData::getQuadraturePenetrationLocator(const BoundaryName & primary,
                                                     const BoundaryName & secondary,
                                                     Order order)
{
  unsigned int primary_id = _mesh.getBoundaryID(primary);
  unsigned int secondary_id = _mesh.getBoundaryID(secondary);

  _subproblem.addGhostedBoundary(primary_id);
  _subproblem.addGhostedBoundary(secondary_id);

  // Generate a new boundary id
  // TODO: Make this better!
  unsigned int base_id = 1e6;
  unsigned int qsecondary_id = secondary_id + base_id;

  _secondary_to_qsecondary[secondary_id] = qsecondary_id;

  PenetrationLocator * pl =
      _penetration_locators[std::pair<unsigned int, unsigned int>(primary_id, qsecondary_id)];

  if (!pl)
  {
    pl = new PenetrationLocator(_subproblem,
                                *this,
                                _mesh,
                                primary_id,
                                qsecondary_id,
                                order,
                                getQuadratureNearestNodeLocator(primary_id, secondary_id));
    _penetration_locators[std::pair<unsigned int, unsigned int>(primary_id, qsecondary_id)] = pl;
  }

  return *pl;
}

NearestNodeLocator &
GeometricSearchData::getNearestNodeLocator(const BoundaryName & primary,
                                           const BoundaryName & secondary)
{
  unsigned int primary_id = _mesh.getBoundaryID(primary);
  unsigned int secondary_id = _mesh.getBoundaryID(secondary);

  _subproblem.addGhostedBoundary(primary_id);
  _subproblem.addGhostedBoundary(secondary_id);

  return getNearestNodeLocator(primary_id, secondary_id);
}

NearestNodeLocator &
GeometricSearchData::getNearestNodeLocator(const unsigned int primary_id,
                                           const unsigned int secondary_id)
{
  NearestNodeLocator * nnl =
      _nearest_node_locators[std::pair<unsigned int, unsigned int>(primary_id, secondary_id)];

  _subproblem.addGhostedBoundary(primary_id);
  _subproblem.addGhostedBoundary(secondary_id);

  if (!nnl)
  {
    nnl = new NearestNodeLocator(_subproblem, _mesh, primary_id, secondary_id);
    _nearest_node_locators[std::pair<unsigned int, unsigned int>(primary_id, secondary_id)] = nnl;
  }

  return *nnl;
}

NearestNodeLocator &
GeometricSearchData::getQuadratureNearestNodeLocator(const BoundaryName & primary,
                                                     const BoundaryName & secondary)
{
  unsigned int primary_id = _mesh.getBoundaryID(primary);
  unsigned int secondary_id = _mesh.getBoundaryID(secondary);

  _subproblem.addGhostedBoundary(primary_id);
  _subproblem.addGhostedBoundary(secondary_id);

  return getQuadratureNearestNodeLocator(primary_id, secondary_id);
}

NearestNodeLocator &
GeometricSearchData::getQuadratureNearestNodeLocator(const unsigned int primary_id,
                                                     const unsigned int secondary_id)
{
  // TODO: Make this better!
  unsigned int base_id = 1e6;
  unsigned int qsecondary_id = secondary_id + base_id;

  _secondary_to_qsecondary[secondary_id] = qsecondary_id;

  return getNearestNodeLocator(primary_id, qsecondary_id);
}

void
GeometricSearchData::generateQuadratureNodes(unsigned int secondary_id,
                                             unsigned int qsecondary_id,
                                             bool reiniting)
{
  // Have we already generated quadrature nodes for this boundary id?
  if (_quadrature_boundaries.find(secondary_id) != _quadrature_boundaries.end())
  {
    if (!reiniting)
      return;
  }
  else
    _quadrature_boundaries.insert(secondary_id);

  const MooseArray<Point> & points_face = _subproblem.assembly(0, 0).qPointsFace();

  ConstBndElemRange & range = *_mesh.getBoundaryElementRange();
  for (const auto & belem : range)
  {
    const Elem * elem = belem->_elem;
    unsigned short int side = belem->_side;
    BoundaryID boundary_id = belem->_bnd_id;

    if (elem->processor_id() == _subproblem.processor_id())
    {
      if (boundary_id == (BoundaryID)secondary_id)
      {
        // All we should need to do here is reinit the underlying libMesh::FE object because that
        // will get us the correct points for the current element and side
        _subproblem.setCurrentSubdomainID(elem, 0);
        _subproblem.assembly(0, 0).reinit(elem, side);

        for (unsigned int qp = 0; qp < points_face.size(); qp++)
          _mesh.addQuadratureNode(elem, side, qp, qsecondary_id, points_face[qp]);
      }
    }
  }
}

void
GeometricSearchData::addElementPairLocator(const unsigned int & interface_id,
                                           std::shared_ptr<ElementPairLocator> epl)
{
  _element_pair_locators[interface_id] = epl;
}

void
GeometricSearchData::updateQuadratureNodes(unsigned int secondary_id)
{
  const MooseArray<Point> & points_face = _subproblem.assembly(0, 0).qPointsFace();

  ConstBndElemRange & range = *_mesh.getBoundaryElementRange();
  for (const auto & belem : range)
  {
    const Elem * elem = belem->_elem;
    unsigned short int side = belem->_side;
    BoundaryID boundary_id = belem->_bnd_id;

    if (elem->processor_id() == _subproblem.processor_id())
    {
      if (boundary_id == (BoundaryID)secondary_id)
      {
        // All we should need to do here is reinit the underlying libMesh::FE object because that
        // will get us the correct points for the current element and side
        _subproblem.setCurrentSubdomainID(elem, 0);
        _subproblem.assembly(0, 0).reinit(elem, side);

        for (unsigned int qp = 0; qp < points_face.size(); qp++)
          (*_mesh.getQuadratureNode(elem, side, qp)) = points_face[qp];
      }
    }
  }
}

void
GeometricSearchData::reinitQuadratureNodes(unsigned int /*secondary_id*/)
{
  // Regenerate the quadrature nodes
  for (const auto & it : _secondary_to_qsecondary)
    generateQuadratureNodes(it.first, it.second, /*reiniting=*/true);
}

void
GeometricSearchData::updateGhostedElems()
{
  for (const auto & nnl_it : _nearest_node_locators)
  {
    NearestNodeLocator * nnl = nnl_it.second;
    nnl->updateGhostedElems();
  }
}
