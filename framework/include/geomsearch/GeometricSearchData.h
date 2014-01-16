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

#ifndef GEOMETRICSEARCHDATA_H
#define GEOMETRICSEARCHDATA_H

#include "MooseTypes.h"

//libmesh includes

#include <map>

//Forward Declarations
class MooseMesh;
class SubProblem;
class PenetrationLocator;
class NearestNodeLocator;

class GeometricSearchData
{
public:
  GeometricSearchData(SubProblem & subproblem, MooseMesh & mesh);
  virtual ~GeometricSearchData();

  PenetrationLocator & getPenetrationLocator(const BoundaryName & master, const BoundaryName & slave, Order order=FIRST);
  PenetrationLocator & getQuadraturePenetrationLocator(const BoundaryName & master, const BoundaryName & slave, Order order=FIRST);
  PenetrationLocator & getMortarPenetrationLocator(const BoundaryName & master, const BoundaryName & slave, Moose::ConstraintSideType side_type, Order order = FIRST);

  NearestNodeLocator & getNearestNodeLocator(const BoundaryName & master, const BoundaryName & slave);
  NearestNodeLocator & getNearestNodeLocator(const unsigned int master_id, const unsigned int slave_id);

  NearestNodeLocator & getQuadratureNearestNodeLocator(const BoundaryName & master, const BoundaryName & slave);
  NearestNodeLocator & getQuadratureNearestNodeLocator(const unsigned int master_id, const unsigned int slave_id);

  NearestNodeLocator & getMortarNearestNodeLocator(const BoundaryName & domain, const BoundaryName & slave, Moose::ConstraintSideType side_type);
  NearestNodeLocator & getMortarNearestNodeLocator(const unsigned int master_id, const unsigned int slave_id, Moose::ConstraintSideType side_type);

  /**
   * Update all of the search objects.
   */
  void update();

  /**
   * Completely redo all geometric search objects.  This should be called when the mesh is adapted.
   */
  void reinit();

//protected:
  SubProblem & _subproblem;
  MooseMesh & _mesh;
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> _penetration_locators;
  std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *> _nearest_node_locators;

protected:

  /// These are _real_ boundaries that have quadrature nodes on them.
  std::set<unsigned int> _quadrature_boundaries;

  /// A mapping of the real boundary id to the slave boundary ids
  std::map<unsigned int, unsigned int> _slave_to_qslave;

  /// These are _real_ boundaries that have quadrature nodes on them.
  std::set<std::pair<unsigned int, unsigned int> > _mortar_boundaries;

  /// A mapping of the real boundary id to the slave boundary ids for mortar spaces
  std::map<unsigned int, unsigned int> _boundary_to_mortarboundary;

private:
  /**
   * Add Quadrature Nodes to the Mesh in support of Quadrature based penetration location and nearest node searching.
   *
   * @param slave_id The actual slave_id (the one in the mesh)
   * @param qslave_id The "fictitious" slave_id that is going to be used for this quadrature nodeset
   */
  void generateQuadratureNodes(unsigned int slave_id, unsigned int qslave_id);

  /**
   * Add Quadrature Nodes to the Mesh in support of mortar based penetration location and nearest node searching.
   *
   * @param slave_id The actual slave_id (the one in the mesh)
   * @param qslave_id The "fictitious" slave_id that is going to be used for this quadrature nodeset
   */
  void generateMortarNodes(unsigned int master_id, unsigned int slave_id, unsigned int qslave_id);

  /**
   * Update the positions of the quadrature nodes.
   */
  void updateQuadratureNodes(unsigned int slave_id);

  /**
   * Completely redo quadrature nodes
   */
  void reinitQuadratureNodes(unsigned int slave_id);

  /**
   * Denotes whether this is the first time the geometric search objects have been updated.
   */
  bool _first;

  /**
   * Update the positions of the quadrature nodes for mortar interfaces
   */
  void updateMortarNodes();

  /**
   * Completely redo quadrature nodes for mortar interfaces
   */
  void reinitMortarNodes();
};

#endif //GEOMETRICSEARCHDATA_H
