//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Reference for B-Spline: https://mat.fsv.cvut.cz/gcg/sbornik/prochazkova.pdf

#pragma once

#include "libMeshReducedNamespace.h"
#include "Moose.h"
#include "MooseError.h"
#include "MooseTypes.h"

namespace MeshSurfaceUtils
{
/**
 * Calculcates the center-of-mass point of a boundary on a mesh.
 * @param boundary boundary name
 * @param mesh_name mesh name (getMesh routine is called within the method)
 * @return Mesh surface center of mass point
 */
libMesh::Point meshSurfaceCOM(const BoundaryName & boundary,
                              const std::unique_ptr<MeshBase> & mesh);

/**
 * Calculates center-of-mass point in a mesh.
 * @param mesh_name mesh name (getMesh routine is called within the method)
 * @return Mesh surface center of mass point
 */
libMesh::Point meshCOM(const std::unique_ptr<MeshBase> & mesh);
}