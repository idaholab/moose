//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/mesh.h"

namespace libMesh
{
class BoundingBox;
}

/**
 * Helper class that computes the intersection of a line segment defined by a point and a direction
 * and a bounding box.
 */
class BoundingBoxIntersectionHelper
{
public:
  /**
   * Constructor.
   * \param bbox The bounding box
   * \param dim The dimension of the bounding box
   */
  BoundingBoxIntersectionHelper(const libMesh::BoundingBox & bbox, const unsigned int dim);

  /**
   * \return The intersection of the segment defined by the point \p point and the direction \p
   * direction. Will return RayTracingCommon::invalid_point if no intersection is found.
   */
  libMesh::Point intersection(const libMesh::Point & point, const libMesh::Point & direction) const;

private:
  /// Dummy communicator for the dummy mesh
  libMesh::Parallel::Communicator _comm;
  /// Dummy mesh that contains a single element used for TraceRayTools intersection methods
  const std::unique_ptr<libMesh::Mesh> _mesh;
  /// hmax for the dummy element
  libMesh::Real _hmax;
};
