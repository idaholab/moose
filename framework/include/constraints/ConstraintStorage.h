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

#ifndef CONSTRAINTSTORAGE_H
#define CONSTRAINTSTORAGE_H

// MOOSE includes
#include "MooseObjectStorage.h"
#include "MooseTypes.h"

class Constraint;
class NodalConstraint;
class NodeFaceConstraint;
class FaceFaceConstraint;

/**
 * Warehouse for storing constraints
 */
class ConstraintStorage : public MooseObjectStorage<Constraint>
{
public:
  ConstraintStorage();

  /**
   * Add Constraint object to the warehouse.
   * @param object A MooseSharedPointer of the object
   * @param tid Not used.
   */
  virtual void addObject(MooseSharedPointer<Constraint> object, THREAD_ID tid = 0);

  ///@{
  /**
   * Access methods for active objects.
   */
  const std::vector<MooseSharedPointer<NodalConstraint> > & getActiveNodalConstraints() const;
  const std::vector<MooseSharedPointer<FaceFaceConstraint> > & getActiveFaceFaceConstraints(const std::string & interface) const;
  const std::vector<MooseSharedPointer<NodeFaceConstraint> > & getActiveNodeFaceConstraints(BoundaryID boundary_id, bool displaced);
  ///@}

  ///@{
  /**
   * Deterimine if active objects exist.
   */
  bool hasActiveNodalConstraints() const;
  bool hasActiveFaceFaceConstraints(const std::string & interface) const;
  bool hasActiveNodeFaceConstraints(BoundaryID boundary_id, bool displaced) const;
  ///@}

  /**
   * Update supplied subdomain and variable coverate containters.
   */
  void subdomainsCovered(std::set<SubdomainID> & subdomains_covered, std::set<std::string> & unique_variables, THREAD_ID tid = 0) const;

  /**
   * Update the various active lists.
   */
  void updateActive(THREAD_ID tid = 0);

protected:

  /// NodalConstraint objects
  MooseObjectStorage<NodalConstraint> _nodal_constraints;

  /// NodeFaceConstraint objects (non-displaced)
  std::map<BoundaryID, MooseObjectStorage<NodeFaceConstraint> > _node_face_constraints;

  /// NodeFaceConstraint objects (displaced)
  std::map<BoundaryID, MooseObjectStorage<NodeFaceConstraint> > _displaced_node_face_constraints;

  /// FaceFaceConstraints
  std::map<std::string, MooseObjectStorage<FaceFaceConstraint> > _face_face_constraints;
};

#endif // CONSTRAINTSTORAGE_H
