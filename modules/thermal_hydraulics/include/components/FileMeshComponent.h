//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeometricalComponent.h"

/**
 * Loads a mesh from an ExodusII file without adding physics.
 */
class FileMeshComponent : public GeometricalComponent
{
public:
  static InputParameters validParams();

  FileMeshComponent(const InputParameters & parameters);

protected:
  virtual void setupMesh() override;

  /**
   * Loads the ExodusII file into the global mesh and returns the subdomain names
   * (before prepending component name)
   */
  std::vector<std::string> buildMesh();

  /// The name of the ExodusII file to load the mesh from
  const FileName & _file_name;
  /// Is the file readable?
  const bool _file_is_readable;

  /// Translation vector for the file mesh
  const Point & _position;
};
