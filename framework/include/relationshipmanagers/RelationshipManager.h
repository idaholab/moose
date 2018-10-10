//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RELATIONSHIPMANAGER_H
#define RELATIONSHIPMANAGER_H

#include "MooseObject.h"
#include "InputParameters.h"
#include "MooseMesh.h"

#include "libmesh/ghosting_functor.h"

// Forward declarations
class RelationshipManager;
class MooseMesh;

template <>
InputParameters validParams<RelationshipManager>();

/**
 * RelationshipManagers are used for describing what kinds of non-local resources are needed for an
 * object's calculations. Non-local resources include geometric element information, or solution
 * information that may be more than a single side-neighbor away in a mesh. This includes
 * physically disconnected elements that might be needed for contact or mortar calculations.
 */
class RelationshipManager : public MooseObject, public libMesh::GhostingFunctor
{
public:
  RelationshipManager(const InputParameters & parameters);

  /**
   * System entry point to trigger the addition of RelationshipManagers to the system. It is not
   * virtual and not intended to be overridden by a derived class.
   * See attachRelationshipManagersInternal()
   */
  void attachRelationshipManagers(Moose::RelationshipManagerType rm_type);

  /**
   * Method for returning relationship manager information (suitable for console output).
   */
  virtual std::string getInfo() const = 0;

  /**
   * Getter for returning the enum type for this RM.
   */
  Moose::RelationshipManagerType getType() const { return _rm_type; }

  //  bool compare(const RelationshipManager & rhs) { return *this == rhs; }

  virtual bool operator==(const RelationshipManager & /*rhs*/) const
  {
    mooseError("Comparison operator required for this RelationshipManager required");
  }

protected:
  /**
   * This method should make the decision of whether or not RMs are needed for the current
   * simulation and attach them to the corresponding libMesh objects. Helper methods exist to
   * attach geometric and algebraic RMs to the right places.
   *
   * This method is called at most once for each "when_type":
   * For GeometricRelationshipManagers it'll be called exactly once. However, "when" it is called
   * depends on the value of the developer-controlled "attach_geometric_early" value.
   * For AlgebraicRelationshipManagers, this method may be called twice, but only once per "when"
   * type. If the RM is able to create its geometric RM early, it should do so and attach it
   * during the normal geometric add time. However, if that's not possible, both the geometric and
   * algebraic RMs can be added during the "late" when time.
   */
  virtual void attachRelationshipManagersInternal(Moose::RelationshipManagerType when_type) = 0;

  /// Reference to the Mesh object
  MooseMesh & _mesh;

  /// Boolean indicating whether this RM can be attached early (e.g. all parameters are known
  /// without the need for inspecting things like variables or other parts of the system that
  /// are not available.
  const bool _attach_geometric_early;

  /// The type of RM this object is. Note that the underlying enum is capable of holding
  /// multiple values simultaneously, so you must use bitwise operators to test values.
  Moose::RelationshipManagerType _rm_type;

private:
  /// Value to keep track of whether the attachRelationshipManagerInternal callback has been
  /// called for this "when_type"
  Moose::RelationshipManagerType _cached_callbacks;

  /// Internal variable indicating whether the mesh is allowed to have elements deleted during
  /// the initial setup phase.
  bool _has_set_remote_elem_removal_flag;
};

#endif /* RELATIONSHIPMANAGER_H */
