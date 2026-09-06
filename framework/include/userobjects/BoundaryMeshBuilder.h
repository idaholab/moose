//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "SurfaceElementSet.h"

#include "libmesh/mesh_base.h"

#include <memory>

/**
 * Owns a saved boundary (surface) mesh and the SurfaceElementSet wrapping it.
 *
 * This is the single object allowed to retrieve a saved mesh via
 * MeshGeneratorSystem::getSavedMesh(): that retrieval std::move()s the mesh out
 * of the mesh-generator system and hard-errors on a second retrieval, so exactly
 * one owner is required. Consumers (point-containment / distance user objects)
 * hold a non-owning reference to this builder instead of re-retrieving the mesh.
 *
 * The whole-mesh SurfaceElementSet is built by the virtual buildDefaultSet()
 * hook. Subclasses that need a different grouping (e.g. per subdomain) build
 * it under their own name instead of overriding this hook, so that
 * surfaceElementSet() still returns a correct whole-mesh set for any caller
 * that requests it via a BoundaryMeshBuilder reference.
 */
class BoundaryMeshBuilder : public GeneralUserObject
{
public:
  static InputParameters validParams();
  BoundaryMeshBuilder(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

  /// The owned boundary mesh. Valid after initialSetup(). Returned mutable
  /// because the TriangleManifold backend validates/improves the mesh in place;
  /// the builder itself does not modify it after initialSetup().
  MeshBase & mesh() const;

  /// The whole-mesh SurfaceElementSet, built lazily on first call. Must be called
  /// only from the single-threaded setup phase (e.g. initialSetup()): the lazy
  /// build is not re-entrant.
  const SurfaceElementSet & surfaceElementSet() const;

protected:
  /// Build the default (whole-mesh) SurfaceElementSet. Not intended to be
  /// overridden: subclasses that need a different grouping should build it
  /// under their own name and leave this hook alone, so that this whole-mesh
  /// set is still correctly built for any caller going through the base class.
  virtual void buildDefaultSet() const;

  /**
   * Whether the boundary mesh is "closed" -- i.e. has no open edges or faces.
   * On a manifold surface mesh this is equivalent to watertightness;
   * non-manifold input is not validated here.
   */
  bool checkWatertightness() const;

  /// The owned boundary mesh, kept alive so SurfaceElement pointers stay valid.
  std::unique_ptr<MeshBase> _mesh;

  /// The whole-mesh SurfaceElementSet, lazily cached on first use.
  mutable std::unique_ptr<SurfaceElementSet> _set;

  /// The name of a mesh saved via a MeshGenerator's `save_mesh_as` parameter.
  const std::string _bnd_mesh_name;

  /// Whether to check watertightness of the mesh (informational only).
  const bool _check_watertightness;

private:
  /// The dimension of the embedding (background) mesh.
  const unsigned int _dim_embedding_mesh;
};
