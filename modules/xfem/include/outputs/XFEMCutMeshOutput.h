//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

// MOOSE includes
#include "FileOutput.h"
#include "UserObjectInterface.h"

// libMesh includes
#include "libmesh/exodusII_io.h"

// Forward declarations
class MeshCutUserObjectBase;

/**
 * Outputs the cutting mesh used by XFEM to an Exodus file.  The output file contains only the mesh,
 * and no solution results.
 */
class XFEMCutMeshOutput : public FileOutput, public UserObjectInterface
{
public:
  XFEMCutMeshOutput(const InputParameters & parameters);

  static InputParameters validParams();

  virtual std::string filename() override;

  virtual void output() override;

private:
  /// The mesh cutting user object
  const MeshCutUserObjectBase & _cutter_uo;

  /// The EquationSystems
  std::unique_ptr<libMesh::EquationSystems> _es;

  /// Exodus for outputting points and values
  std::unique_ptr<libMesh::ExodusII_IO> _exodus_io;
};
