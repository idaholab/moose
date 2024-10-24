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

class FileMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  FileMesh(const InputParameters & parameters);
  FileMesh(const FileMesh & other_mesh);
  virtual ~FileMesh(); // empty dtor required for unique_ptr with forward declarations

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

  void read(const std::string & file_name);

  // Get/Set Filename (for meshes read from a file)
  void setFileName(const std::string & file_name) { _file_name = file_name; }
  virtual std::string getFileName() const override { return _file_name; }

protected:
  /// the file_name from whence this mesh came
  std::string _file_name;

  /// Auxiliary object for restart
  std::unique_ptr<libMesh::ExodusII_IO> _exreader;

  /// The requested dimension of the mesh. For some file meshes, this is not required may be implied
  /// from the element type(s).
  const unsigned int _dim;
};
