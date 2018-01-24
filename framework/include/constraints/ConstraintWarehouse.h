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

// MOOSE includes
#include "MooseObjectWarehouse.h"
#include "MooseTypes.h"

class Constraint;
class NodalConstraint;
class NodeFaceConstraint;
class FaceFaceConstraint;
class ElemElemConstraint;

/**
 * Warehouse for storing constraints
 */
class ConstraintWarehouse : public MooseObjectWarehouse<Constraint>
{
public:
  ConstraintWarehouse();

  /**
   * Add Constraint object to the warehouse.
   * @param object A std::shared_ptr of the object
   * @param tid Not used.
   */
  void addObject(std::shared_ptr<Constraint> object, THREAD_ID tid = 0);

  ///@{
  /**
   * Access methods for active objects.
   */
  const std::vector<std::shared_ptr<NodalConstraint>> & getActiveNodalConstraints() const;
  const std::vector<std::shared_ptr<FaceFaceConstraint>> &
  getActiveFaceFaceConstraints(const std::string & interface) const;
  const std::vector<std::shared_ptr<ElemElemConstraint>> &
  getActiveElemElemConstraints(InterfaceID interface_id) const;
  const std::vector<std::shared_ptr<NodeFaceConstraint>> &
  getActiveNodeFaceConstraints(BoundaryID boundary_id, bool displaced) const;
  ///@}

  ///@{
  /**
   * Deterimine if active objects exist.
   */
  bool hasActiveNodalConstraints() const;
  bool hasActiveFaceFaceConstraints(const std::string & interface) const;
  bool hasActiveElemElemConstraints(const InterfaceID interface_id) const;
  bool hasActiveNodeFaceConstraints(BoundaryID boundary_id, bool displaced) const;
  ///@}

  /**
   * Update supplied subdomain and variable coverate containters.
   */
  void subdomainsCovered(std::set<SubdomainID> & subdomains_covered,
                         std::set<std::string> & unique_variables,
                         THREAD_ID tid = 0) const;

  /**
   * Update the various active lists.
   */
  void updateActive(THREAD_ID tid = 0);

protected:
  /// NodalConstraint objects
  MooseObjectWarehouse<NodalConstraint> _nodal_constraints;

  /// NodeFaceConstraint objects (non-displaced)
  std::map<BoundaryID, MooseObjectWarehouse<NodeFaceConstraint>> _node_face_constraints;

  /// NodeFaceConstraint objects (displaced)
  std::map<BoundaryID, MooseObjectWarehouse<NodeFaceConstraint>> _displaced_node_face_constraints;

  /// FaceFaceConstraints
  std::map<std::string, MooseObjectWarehouse<FaceFaceConstraint>> _face_face_constraints;

  /// ElemElemConstraints
  std::map<unsigned int, MooseObjectWarehouse<ElemElemConstraint>> _element_constraints;
};

#endif // CONSTRAINTWAREHOUSE_H
