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
 * objects calculations. Non-local resources include geometric element information, or solution
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

protected:
  /**
   * Method to setup the RelationshipManager for use in the simulation. It is called exactly
   * once for each type of functor object that this RelationshipManager is capable of creating.
   * For GeometricRelationshipManagers, it is called exactly once. For
   * AlgebraicRelationshipManagers, it is called twice, once for the geometric RM and once for
   * the algebraic RM. This method should make the decision of whether or not a RM is needed
   * for the current simulation and attach it to the right libMesh object. Note the helper
   * methods available in the two major types of RMs.
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
