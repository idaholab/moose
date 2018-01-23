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
 * needed for an objects calculations. Non-local resources include geometric element information,
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

  /// Problem pointer
  FEProblem * _problem;
};

#endif /* ALGEBRAICRELATIONSHIPMANAGER_H */
