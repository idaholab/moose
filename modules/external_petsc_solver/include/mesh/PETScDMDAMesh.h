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

/**
 * Generate a parallel (distributed) mesh from PETSc DMDA.
 * DMDA could be passed in from a PETSc application
 * or created on the fly. Note that this mesh object does not have one layer of
 * ghost elements. It is designed for holding the solution from an external PETSc
 * application. And then the solution can be coupled to other MOOSE-based applications
 * using the existing MultiApp transfers.
 */
class PETScDMDAMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  PETScDMDAMesh(const InputParameters & parameters);
  PETScDMDAMesh(const PETScDMDAMesh & /* other_mesh */) = default;

  // No copy
  PETScDMDAMesh & operator=(const PETScDMDAMesh & other_mesh) = delete;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

protected:
  DM _dmda;
};
