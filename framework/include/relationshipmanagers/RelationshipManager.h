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
  virtual ~RelationshipManager() {}

  /**
   * Method to setup the RelationshipManager for use in the simulation.
   */
  virtual void init() = 0;

  /**
   * Method to indicate whether the status of the relationship manager.
   *
   * @return bool Indicates whether the relationship manager should be made active (i.e. attached to
   * the mesh or dof_map in libMesh).
   */
  virtual bool isActive() const { return false; }

  /**
   * Method for returning relationship manager information (suitable for console output).
   */
  virtual std::string getInfo() const = 0;

protected:
  MooseMesh & _mesh;
};

#endif /* RELATIONSHIPMANAGER_H */
