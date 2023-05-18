//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
class MeshCut2DUserObjectBase;

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

  virtual void output(const ExecFlagType & type) override;

private:
  /// The mesh cutting user object
  const MeshCut2DUserObjectBase & _cutter_uo;

  /// The EquationSystems
  std::unique_ptr<EquationSystems> _es;

  /// Exodus for outputing points and values
  std::unique_ptr<ExodusII_IO> _exodus_io;
};
