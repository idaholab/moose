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

#include "Warehouse.h"

class Constraint;
class NodalConstraint;
class NodeFaceConstraint;
class FaceFaceConstraint;

/**
 * Warehouse for storing constraints
 */
class ConstraintWarehouse : public Warehouse<Constraint>
{
public:
  ConstraintWarehouse();
  virtual ~ConstraintWarehouse();

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  void addNodalConstraint(MooseSharedPointer<NodalConstraint> nfc);

  void addNodeFaceConstraint(unsigned int slave, unsigned int master, MooseSharedPointer<NodeFaceConstraint> nfc);

  void addFaceFaceConstraint(const std::string & name, MooseSharedPointer<FaceFaceConstraint> ffc);

  std::vector<NodalConstraint *> & getNodalConstraints();

  std::vector<NodeFaceConstraint *> & getNodeFaceConstraints(BoundaryID boundary_id);
  std::vector<NodeFaceConstraint *> & getDisplacedNodeFaceConstraints(BoundaryID boundary_id);

  std::vector<FaceFaceConstraint *> & getFaceFaceConstraints(const std::string & name);

  void subdomainsCovered(std::set<SubdomainID> & subdomains_covered, std::set<std::string> & unique_variables) const;

protected:
  /**
   * We are using MooseSharedPointer to handle the cleanup of the pointers at the end of execution.
   * This is necessary since several warehouses might be sharing a single instance of a MooseObject.
   */
  std::vector<MooseSharedPointer<Constraint> > _all_ptrs;

  /// nodal constraints on a boundary
  std::vector<NodalConstraint *> _nodal_constraints;

  std::map<BoundaryID, std::vector<NodeFaceConstraint *> > _node_face_constraints;
  std::map<BoundaryID, std::vector<NodeFaceConstraint *> > _displaced_node_face_constraints;

  std::map<std::string, std::vector<FaceFaceConstraint *> > _face_face_constraints;

  // We can't use "auto", but these typedefs make parsing for loops much easier for humans...
  typedef std::vector<NodalConstraint *>::const_iterator NodalConstraintIter;
  typedef std::map<BoundaryID, std::vector<NodeFaceConstraint *> >::const_iterator NodeFaceIter;
  typedef std::map<std::string, std::vector<FaceFaceConstraint *> >::const_iterator FaceFaceIter;
};

#endif // CONSTRAINTWAREHOUSE_H
