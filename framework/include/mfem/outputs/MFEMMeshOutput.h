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

#include "FileOutput.h"
#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

/**
 * Writes an MFEM mesh in the native MFEM mesh format.
 */
class MFEMMeshOutput : public FileOutput
{
public:
  static InputParameters validParams();
  MFEMMeshOutput(const InputParameters & parameters);
  std::string filename() override;

protected:
  void output() override;

  /// Mesh to write: the problem mesh or one of its submeshes. It is gathered onto a single process
  /// when written.
  mfem::ParMesh & _pmesh;

  /// Whether and how the mesh elements should be reordered prior to output.
  const MooseEnum _ordering;

  /// Number of significant digits to write in the ASCII output file.
  const int _precision;
};

#endif
