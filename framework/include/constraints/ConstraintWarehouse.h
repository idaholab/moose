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
#include "MooseTypes.h"
#include "MooseObjectWarehouse.h"

class Constraint;
class NodalConstraint;
class NodeFaceConstraint;
class FaceFaceConstraint;

/**
 * Warehouse for storing constraints
 */
class ConstraintWarehouse : public MooseObjectWarehouse<Constraint>
{
public:
  ConstraintWarehouse();
  virtual ~ConstraintWarehouse();

  /**
   * Adds a constraint to the Warehouse.
   */
  virtual void addObject(MooseSharedPointer<Constraint> constraint, THREAD_ID tid = 0);

  ///@{
  /**
   * Methods to test if active Constraint objects exist.
   */
  bool hasActiveNodalConstraints() const;
  bool hasActiveNodeFaceConstraints(BoundaryID boundary_id, bool displaced = false) const;
  bool hasActiveFaceFaceConstraints(const std::string & name) const;
  ///@}

  ///@{
  /**
   * Methods for accessing the active Constraint objects.
   */
  const std::vector<MooseSharedPointer<NodalConstraint> > & getActiveNodalConstraints() const;
  const std::vector<MooseSharedPointer<NodeFaceConstraint> > & getActiveNodeFaceConstraints(BoundaryID boundary_id, bool displaced = false) const;
  const std::vector<MooseSharedPointer<FaceFaceConstraint> > & getActiveFaceFaceConstraints(const std::string & name) const;
  ///@}

  /**
   * Update ths list of subdomains with constraints.
   */
  void subdomainsCovered(std::set<SubdomainID> & subdomains_covered, std::set<std::string> & unique_variables) const;

  /**
   * Update the active lists, for all types stored.
   */
  virtual void updateActive(THREAD_ID tid = 0);

protected:

  /// Storage for NodalConstraint objects
  MooseObjectStorage<NodalConstraint> _nodal_constraints;

  /// Storage for NodeFaceConstraint objects (non-displaced)
  std::map<BoundaryID, MooseObjectStorage<NodeFaceConstraint> > _node_face_constraints;

  /// Storage for NodeFaceConstraint objects (displaced)
  std::map<BoundaryID, MooseObjectStorage<NodeFaceConstraint> > _displaced_node_face_constraints;

  /// Storage for FaceFaceConstraints
  std::map<std::string, MooseObjectStorage<FaceFaceConstraint> > _face_face_constraints;

};

#endif // CONSTRAINTWAREHOUSE_H
