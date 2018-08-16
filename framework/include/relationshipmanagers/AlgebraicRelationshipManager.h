//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ALGEBRAICRELATIONSHIPMANAGER_H
#define ALGEBRAICRELATIONSHIPMANAGER_H

#include "GeometricRelationshipManager.h"
#include "LazyCoupleable.h"

// Forward declarations
class AlgebraicRelationshipManager;
class FEProblem;

template <>
InputParameters validParams<AlgebraicRelationshipManager>();

/**
 * AlgebraicRelationshipManagers are used for describing what kinds of non-local resources are
 * needed for an object's calculations. Non-local resources include geometric element information,
 * and solution information that may be more than a single side-neighbor away in a mesh. This
 * includes physically disconnected elements that might be needed for contact or mortar
 * calculations. AlgebraicRelationshipManagers should also be attached to the Mesh for full
 * DistributedMesh capability.
 */
class AlgebraicRelationshipManager : public GeometricRelationshipManager, public LazyCoupleable
{
public:
  AlgebraicRelationshipManager(const InputParameters & parameters);

protected:
  /**
   * Helper method for attaching ghosting functors to the relevant EquationSystem's DofMap.
   */
  void attachAlgebraicFunctorHelper(GhostingFunctor & gf) const;
};

#endif /* ALGEBRAICRELATIONSHIPMANAGER_H */
