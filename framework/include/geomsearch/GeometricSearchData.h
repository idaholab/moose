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

//libmesh includes
#include "MooseMesh.h"

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

  NearestNodeLocator & getNearestNodeLocator(const BoundaryName & master, const BoundaryName & slave);
  NearestNodeLocator & getNearestNodeLocator(const unsigned int master_id, const unsigned int slave_id);

  NearestNodeLocator & getQuadratureNearestNodeLocator(const BoundaryName & master, const BoundaryName & slave);
  NearestNodeLocator & getQuadratureNearestNodeLocator(const unsigned int master_id, const unsigned int slave_id);

  /**
   * Update all of the search objects.
   *
   * This is probably getting called because the mesh changed in some way.
   */
  void update();

//protected:
  SubProblem & _subproblem;
  MooseMesh & _mesh;
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *> _penetration_locators;
  std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *> _nearest_node_locators;

protected:

  /// These are _real_ boundaries that have quadrature nodes on them.
  std::set<unsigned int> _quadrature_boundaries;

private:
  /**
   * Add Quadrature Nodes to the Mesh in support of Quadrature based penetration location and nearest node searching.
   *
   * @param slave_id The actual slave_id (the one in the mesh)
   * @param qslave_id The "fictitious" slave_id that is going to be used for this quadrature nodeset
   */
  void generateQuadratureNodes(unsigned int slave_id, unsigned int qslave_id);

  /**
   * Update the positions of the quadrature nodes.
   */
  void updateQuadratureNodes(unsigned int slave_id);
};

#endif //GEOMETRICSEARCHDATA_H
