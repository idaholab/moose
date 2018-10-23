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
   * Called before this RM is attached.  Will only be called once.
   */
  void init()
  {
    _inited = true;
    internalInit();
  }

  /**
   * Whether or not this RM has been inited
   */
  bool inited() { return _inited; }

  /**
   * Method for returning relationship manager information (suitable for console output).
   */
  virtual std::string getInfo() const = 0;

  /**
   * Getter for returning the enum type for this RM.
   */
  Moose::RelationshipManagerType getType() const { return _rm_type; }

  /**
   * Check to see if an RM is of a given type
   *
   * This is here so that the boolean logic doesn't have to get spread everywhere in the world
   */
  bool isType(const Moose::RelationshipManagerType & type) { return (_rm_type & type) == type; }

  virtual bool operator==(const RelationshipManager & /*rhs*/) const
  {
    mooseError("Comparison operator required for this RelationshipManager required");
  }

  /**
   * Whether or not this RM can be attached to the Mesh early if it's geometric.
   *
   * Note: this is unused if this RM is purely Algebraic
   */
  bool attachGeometricEarly() { return _attach_geometric_early; }

protected:
  /**
   * Called before this RM is attached.  Only called once
   */
  virtual void internalInit() = 0;

  /// Whether or not this has been initialized
  bool _inited = false;

  /// Reference to the Mesh object
  MooseMesh & _mesh;

  /// Boolean indicating whether this RM can be attached early (e.g. all parameters are known
  /// without the need for inspecting things like variables or other parts of the system that
  /// are not available.
  const bool _attach_geometric_early;

  /// The type of RM this object is. Note that the underlying enum is capable of holding
  /// multiple values simultaneously, so you must use bitwise operators to test values.
  Moose::RelationshipManagerType _rm_type;
};

#endif /* RELATIONSHIPMANAGER_H */
