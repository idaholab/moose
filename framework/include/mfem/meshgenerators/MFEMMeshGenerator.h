//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MeshGenerator.h"

/**
 * Abstract base class for MFEM-native mesh generators. Sits inside the standard
 * MeshGenerator hierarchy so MFEM generators register, chain, and compose like any
 * other mesh generator.
 *
 * Subclasses implement generateMFEMMesh() instead of generate(). The base class
 * wraps the returned mfem::Mesh in an MFEMMeshCarrier so it can travel through the
 * libMesh-typed pipeline. Inputs from upstream generators are unwrapped via
 * getMFEMInputMesh().
 *
 * generate() is marked final so subclasses cannot accidentally bypass the carrier
 * wrapping.
 */
class MFEMMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  MFEMMeshGenerator(const InputParameters & parameters);

  /// Sealed. Calls generateMFEMMesh() and wraps the result in an MFEMMeshCarrier.
  std::unique_ptr<MeshBase> generate() final override;

protected:
  /// Implement this to produce the mfem::Mesh for this generator.
  virtual mfem::Mesh generateMFEMMesh() = 0;

  /**
   * Retrieve and unwrap the mfem::Mesh from an upstream generator's MFEMMeshCarrier.
   *
   * @param mesh_ref  A reference to the unique_ptr<MeshBase> obtained by calling getMesh()
   *                  in the subclass constructor. The pointer is moved out (nulled) here,
   *                  satisfying the pipeline's requirement that all requested meshes are
   *                  released after generate().
   *
   * Throws a clear error if the upstream output is not an MFEMMeshCarrier (e.g. if
   * a libMesh generator was accidentally wired into an MFEM chain).
   */
  mfem::Mesh getMFEMInputMesh(std::unique_ptr<MeshBase> & mesh_ref);
};

#endif
