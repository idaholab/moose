//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FILEMESH_H
#define FILEMESH_H

#include "MooseMesh.h"

// forward declaration
class FileMesh;

template <>
InputParameters validParams<FileMesh>();

class FileMesh : public MooseMesh
{
public:
  FileMesh(const InputParameters & parameters);
  FileMesh(const FileMesh & other_mesh);
  virtual ~FileMesh(); // empty dtor required for unique_ptr with forward declarations

  virtual MooseMesh & clone() const override;

  virtual void buildMesh() override;

  void read(const std::string & file_name);
  virtual ExodusII_IO * exReader() const override { return _exreader.get(); }

  // Get/Set Filename (for meshes read from a file)
  void setFileName(const std::string & file_name) { _file_name = file_name; }
  const std::string & getFileName() const { return _file_name; }

protected:
  /// the file_name from whence this mesh came
  std::string _file_name;
  /// Auxiliary object for restart
  std::unique_ptr<ExodusII_IO> _exreader;
};

#endif // FILEMESH_H
