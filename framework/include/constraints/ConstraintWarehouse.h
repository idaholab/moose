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

// Moose includes
#include "NodeFaceConstraint.h"

// system includes
#include <map>
#include <vector>

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

  void addNodeFaceConstraint(unsigned int slave, unsigned int master, NodeFaceConstraint * nfc);

  std::vector<NodeFaceConstraint *> & getNodeFaceConstraints(unsigned int boundary_id);
  std::vector<NodeFaceConstraint *> & getDisplacedNodeFaceConstraints(unsigned int boundary_id);

protected:
  std::map<unsigned int, std::vector<NodeFaceConstraint *> > _node_face_constraints;

  std::map<unsigned int, std::vector<NodeFaceConstraint *> > _displaced_node_face_constraints;
};

#endif // CONSTRAINTWAREHOUSE_H
