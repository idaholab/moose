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
#include "MFEMMesh.h"
#include "MFEMParMesh.h"
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
 * ExclusiveMFEMMesh
 *
 * ExclusiveMFEMMesh inherits a MOOSE mesh class which allows us to work with
 * other MOOSE objects. It contains pointers to an MFEM serial and MFEM parallel
 * mesh.
 */
class ExclusiveMFEMMesh : public FileMesh
{
public:
  static InputParameters validParams();

  ExclusiveMFEMMesh(const InputParameters & parameters);

  virtual ~ExclusiveMFEMMesh();

  std::unique_ptr<MooseMesh> safeClone() const override;

  /**
   * Accessors for the _mfem_mesh and _mfem_par_mesh objects. If the objects have
   * not been build, the methods will call the appropriate protected methods to
   * build them.
   */
  MFEMMesh & getMFEMMesh();
  MFEMParMesh & getMFEMParMesh();

  /**
   * Calls buildDummyMesh.
   */
  void buildMesh() override;

  /**
   * Override in derived classes.
   */
  virtual inline int getLocalMFEMNodeId(const int libmesh_global_node_id) const
  {
    mooseError("Not implemented.");
  }

  virtual inline int getLibmeshGlobalNodeId(const int mfem_local_node_id) const
  {
    mooseError("Not implemented.");
  }

protected:
  /**
   * Builds a placeholder mesh when no MOOSE mesh is required.
   */
  void buildDummyMesh();

  /**
   * Builds an MFEMMesh object from a file. Override in derived classes.
   */
  virtual void buildMFEMMesh();

  /**
   * Builds an MFEMParMesh object from a file. Override in derived classes.
   */
  virtual void buildMFEMParMesh();

  /**
   * Smart pointers to MFEMMesh and MFEMParMesh objects. Do not access directly.
   * Use the accessors instead.
   */
  std::shared_ptr<MFEMMesh> _mfem_mesh;
  std::shared_ptr<MFEMParMesh> _mfem_par_mesh;
};
