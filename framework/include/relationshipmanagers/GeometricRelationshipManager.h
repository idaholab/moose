//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
