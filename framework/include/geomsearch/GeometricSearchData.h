//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseTypes.h"

// libMesh includes
#include "libmesh/enum_order.h"

// C++ includes
#include <map>

// Forward Declarations
class MooseMesh;
class SubProblem;
class PenetrationLocator;
class NearestNodeLocator;
class ElementPairLocator;

class GeometricSearchData
{
public:
  /// Used to select groups of geometric search objects to update
  enum GeometricSearchType
  {
    ALL,
    NEAREST_NODE,
    PENETRATION,
    QUADRATURE,
    ELEMENTPAIR
  };

  GeometricSearchData(SubProblem & subproblem, MooseMesh & mesh);
  virtual ~GeometricSearchData();

  PenetrationLocator & getPenetrationLocator(const BoundaryName & primary,
                                             const BoundaryName & secondary,
                                             libMesh::Order order = libMesh::FIRST);
  PenetrationLocator & getQuadraturePenetrationLocator(const BoundaryName & primary,
                                                       const BoundaryName & secondary,
                                                       libMesh::Order order = libMesh::FIRST);

  NearestNodeLocator & getNearestNodeLocator(const BoundaryName & primary,
                                             const BoundaryName & secondary);
  NearestNodeLocator & getNearestNodeLocator(const BoundaryID primary_id,
                                             const BoundaryID secondary_id);

  NearestNodeLocator & getQuadratureNearestNodeLocator(const BoundaryName & primary,
                                                       const BoundaryName & secondary);
  NearestNodeLocator & getQuadratureNearestNodeLocator(const BoundaryID primary_id,
                                                       const BoundaryID secondary_id);

  const std::map<std::pair<BoundaryID, BoundaryID>, PenetrationLocator *> &
  getPenetrationLocators() const
  {
    return _penetration_locators;
  }

  void addElementPairLocator(BoundaryID interface_id, std::shared_ptr<ElementPairLocator> epl);

  /**
   * Update all of the search objects.
   */
  void update(GeometricSearchType type = ALL);

  /**
   * Completely redo all geometric search objects.  This should be called when the mesh is adapted.
   */
  void reinit();

  /**
   * Clear out the Penetration Locators so they will redo the search.
   */
  void clearNearestNodeLocators();

  /**
   * Maximum percentage through the search patch that any NearestNodeLocator had to look.
   *
   * As this goes towards 1.0 it's indicative of needing to rebuild the patches.
   */
  Real maxPatchPercentage();

  /**
   * Updates the list of ghosted elements at the start of each time step for the nonlinear
   * iteration patch update strategy.
   */
  void updateGhostedElems();

  // protected:
  SubProblem & _subproblem;
  MooseMesh & _mesh;
  std::map<std::pair<BoundaryID, BoundaryID>, PenetrationLocator *> _penetration_locators;
  std::map<std::pair<BoundaryID, BoundaryID>, NearestNodeLocator *> _nearest_node_locators;
  std::map<BoundaryID, std::shared_ptr<ElementPairLocator>> _element_pair_locators;

protected:
  /// These are _real_ boundaries that have quadrature nodes on them.
  std::set<BoundaryID> _quadrature_boundaries;

  /// A mapping of the real boundary id to the secondary boundary ids
  std::map<BoundaryID, BoundaryID> _secondary_to_qsecondary;

private:
  /**
   * Add Quadrature Nodes to the Mesh in support of Quadrature based penetration location and
   * nearest node searching.
   *
   * @param secondary_id The actual secondary_id (the one in the mesh)
   * @param qsecondary_id The "fictitious" secondary_id that is going to be used for this quadrature
   * nodeset
   * @param reiniting Whether we are reinitializing, e.g. whether we need to re-generate q-nodes
   */
  void generateQuadratureNodes(const BoundaryID secondary_id,
                               const BoundaryID qsecondary_id,
                               bool reiniting = false);

  /**
   * Update the positions of the quadrature nodes.
   */
  void updateQuadratureNodes(const BoundaryID secondary_id);

  /**
   * Completely redo quadrature nodes
   */
  void reinitQuadratureNodes(const BoundaryID secondary_id);

  /**
   * Denotes whether this is the first time the geometric search objects have been updated.
   */
  bool _first;
};
