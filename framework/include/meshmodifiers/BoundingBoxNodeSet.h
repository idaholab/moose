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

#ifndef BOUNDINGBOXNODESET_H
#define BOUNDINGBOXNODESET_H

#include "MeshModifier.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

// Forward Declaration
class BoundingBoxNodeSet;

template <>
InputParameters validParams<BoundingBoxNodeSet>();

/**
 * Selects a set of nodes and assigns a nodeset name to them based on
 * the bounding box specified. Can select nodes "inside" or "outside"
 * the bounding box.
 */
class BoundingBoxNodeSet : public MeshModifier
{
public:
  BoundingBoxNodeSet(const InputParameters & parameters);

protected:
  virtual void modify() override;

private:
  /// Select nodes on the 'inside' or the 'outside' of the bounding box
  MooseEnum _location;

  /// Bounding box for testing element centroids against. Note that
  MeshTools::BoundingBox _bounding_box;
};

#endif // BOUNDINGBOXNODESET_H
