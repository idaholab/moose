//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"

class PETScDMDAMesh;

template <>
InputParameters validParams<PETScDMDAMesh>();

/**
 * Generate a parallel (distributed) mesh from PETSc DMDA.
 * DMDA could be passed in from an application such as ExternalPetscSolverApp
 * or created on the fly. Note that this mesh object does not have one layer of
 * ghost elements. It is designed for holding the solution from an external PETSc
 * application. And then the solution can be coupled to other MOOSE-based applications
 * using the existing MultiApp transfers.
 */
class PETScDMDAMesh : public MooseMesh
{
public:
  PETScDMDAMesh(const InputParameters & parameters);
  PETScDMDAMesh(const PETScDMDAMesh & /* other_mesh */) = default;

#if LIBMESH_HAVE_PETSC
  ~PETScDMDAMesh()
  {
    if (_need_to_destroy_dmda)
      DMDestroy(&_dmda);
  }
#endif
  // No copy
  PETScDMDAMesh & operator=(const PETScDMDAMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;
  virtual Real getMinInDimension(unsigned int component) const override;
  virtual Real getMaxInDimension(unsigned int component) const override;

protected:
  /// The dimension of the mesh
  MooseEnum _dim;

  /// Number of elements in x, y, z direction
  dof_id_type _nx, _ny, _nz;

  /// The min/max values for x,y,z component
  Real _xmin, _xmax, _ymin, _ymax, _zmin, _zmax;

  /// The type of element to build
  ElemType _elem_type;

  /// If DMDA is created on the fly, we should destroy it.
  bool _need_to_destroy_dmda;
#if LIBMESH_HAVE_PETSC
  /// Mesh object
  DM _dmda;
#endif
};

