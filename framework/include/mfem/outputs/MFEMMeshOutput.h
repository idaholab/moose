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
#include "MFEMProblem.h"

/**
 * Class for output MFEM mesh in native format.
 */
class MFEMMeshOutput : public FileOutput
{
public:
  static InputParameters validParams();
  MFEMMeshOutput(const InputParameters & parameters);
  std::string filename() override;

protected:
  void output() override;

  /// Mesh set of output variables are defined on. May differ from main problem mesh if SubMesh
  /// variables are in use. Always handled as a serial mesh.
  mfem::ParMesh & _pmesh;

  /// Whether and how the mesh elements should be reordered prior to output.
  int _ordering;

  /// Number of decimal places to include in the ASCII output file.
  int _precision;
};

#endif
