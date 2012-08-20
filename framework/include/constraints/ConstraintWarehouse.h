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

#ifndef CONSTRAINTWAREHOUSE_H
#define CONSTRAINTWAREHOUSE_H

// system includes
#include <map>
#include <vector>

#include "MooseTypes.h"

class NodalConstraint;
class NodeFaceConstraint;

/**
 * Warehouse for storing boundary conditions (for non-linear variables)
 */
class ConstraintWarehouse
{
public:
  ConstraintWarehouse();
  virtual ~ConstraintWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  void addNodalConstraint(NodalConstraint * nfc);

  void addNodeFaceConstraint(unsigned int slave, unsigned int master, NodeFaceConstraint * nfc);

  std::vector<NodalConstraint *> & getNodalConstraints();

  std::vector<NodeFaceConstraint *> & getNodeFaceConstraints(BoundaryID boundary_id);
  std::vector<NodeFaceConstraint *> & getDisplacedNodeFaceConstraints(BoundaryID boundary_id);

protected:
  /// nodal constraints on a boundary
  std::vector<NodalConstraint *> _nodal_constraints;

  std::map<BoundaryID, std::vector<NodeFaceConstraint *> > _node_face_constraints;
  std::map<BoundaryID, std::vector<NodeFaceConstraint *> > _displaced_node_face_constraints;
};

#endif // CONSTRAINTWAREHOUSE_H
