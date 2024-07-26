//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "FileMesh.h"
#include "memory"
#include "libmesh/elem.h"
#include "libmesh/enum_io_package.h"
#include "libmesh/equation_systems.h"
#include "libmesh/face_quad4.h"
#include "libmesh/ignore_warnings.h"
#include "libmesh/libmesh_config.h"
#include "libmesh/mesh_base.h"
#include "libmesh/mesh_input.h"
#include "libmesh/node.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/system.h"
#include "libmesh/vtk_io.h"
#include "mfem.hpp"

/**
 * MFEMMesh
 *
 * MFEMMesh inherits a MOOSE mesh class which allows us to work with
 * other MOOSE objects. It contains a pointer to the parallel MFEM mesh.
 */
class MFEMMesh : public FileMesh
{
public:
  static InputParameters validParams();

  MFEMMesh(const InputParameters & parameters);

  virtual ~MFEMMesh();

  std::unique_ptr<MooseMesh> safeClone() const override;

  /**
   * Accessors for the _mfem_par_mesh object. If the mesh has
   * not been build, the methods will call the appropriate protected methods to
   * build them.
   */
  mfem::ParMesh & getMFEMParMesh() { return *_mfem_par_mesh; };
  /**
   * Build MFEM ParMesh and a placeholder MOOSE mesh.
   */
  void buildMesh() override;

protected:
  /**
   * Builds a placeholder mesh when no MOOSE mesh is required.
   */
  void buildDummyMooseMesh();

private:
  /**
   * Smart pointers to mfem::ParMesh object. Do not access directly.
   * Use the accessors instead.
   */
  std::shared_ptr<mfem::ParMesh> _mfem_par_mesh{nullptr};
};
