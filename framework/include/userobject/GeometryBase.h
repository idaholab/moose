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

#ifndef GEOMETRYBASE_H
#define GEOMETRYBASE_H

#include "GeneralUserObject.h"

class GeometryBase;

template <>
InputParameters validParams<GeometryBase>();

/**
 * Base class for userobjects that snap nodes to a defined geometry when
 * adaptivity happens.
 */
class GeometryBase : public GeneralUserObject
{
public:
  GeometryBase(const InputParameters & parameters);

  virtual void initialize() final;
  virtual void execute() final;
  virtual void finalize() final;

  virtual void meshChanged() final;

protected:
  /**
   * Override this method in derived classes to implement a specific geometry.
   * The method takes a writable reference to a node. Set the position of the
   * node to the closest point on the surface of the true geometry.
   */
  virtual void snapNode(Node & node) = 0;

  /// Reference to teh current simulation mesh
  MooseMesh & _mesh;

  /// List of boundaries (or node sets) that will be snapped to a geometry
  const std::vector<BoundaryID> _boundary_ids;
};

#endif // GEOMETRYBASE_H
