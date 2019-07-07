//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

#ifdef LIBMESH_HAVE_TRIANGLE
#include "libmesh/mesh_triangle_interface.h"
#endif

class TriangleMesher;

template <>
InputParameters validParams<TriangleMesher>();

/**
 * Uses Triangle to mesh an enclosed area.
 */
class TriangleMesher : public MeshGenerator
{
public:
  TriangleMesher(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::vector<std::unique_ptr<MeshBase> *> _mesh_ptrs;

#ifdef LIBMESH_HAVE_TRIANGLE
  std::vector<TriangleInterface::Hole *> _holes;

  std::vector<TriangleInterface::Region *> _regions;
#endif
};
