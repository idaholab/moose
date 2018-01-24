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

#ifndef GEOMETRICRELATIONSHIPMANAGER_H
#define GEOMETRICRELATIONSHIPMANAGER_H

#include "RelationshipManager.h"

// Forward declarations
class GeometricRelationshipManager;

template <>
InputParameters validParams<GeometricRelationshipManager>();

/**
 * GeometricRelationshipManagers are used for describing what kinds of non-local resources are
 * needed for an object's calculations. Non-local resources include geometric element information,
 * such as elements or boundaries that may be more than a single side-neighbor away within the mesh.
 * This includes physically disconnected elements that might be needed for contact or mortar
 * calculations.
 */
class GeometricRelationshipManager : public RelationshipManager
{
public:
  GeometricRelationshipManager(const InputParameters & parameters);

protected:
  /**
   * Helper method for attaching ghosting functors to the Mesh object.
   */
  void attachGeometricFunctorHelper(GhostingFunctor & gf) const;
};

#endif /* GEOMETRICRELATIONSHIPMANAGER_H */
