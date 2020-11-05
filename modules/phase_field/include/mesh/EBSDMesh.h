//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneratedMesh.h"
#include "EBSDMeshGenerator.h"

#include <array>

/**
 * Mesh generated from parameters
 */
class EBSDMesh : public GeneratedMesh
{
public:
  static InputParameters validParams();

  EBSDMesh(const InputParameters & parameters);
  virtual ~EBSDMesh();

  virtual void buildMesh();

  struct EBSDMeshGeometry
  {
    // grid spacing
    std::array<Real, 3> d;
    // grid origin
    std::array<Real, 3> min;
    // mesh dimension
    unsigned int dim;
    // grid size
    std::array<unsigned int, 3> n;
  };

  // Interface functions for the EBSDReader
  const EBSDMeshGenerator::Geometry & getEBSDGeometry() const { return _geometry; }
  const std::string & getEBSDFilename() const { return _filename; }

protected:
  /// Read the EBSD data file header
  void readEBSDHeader();

  /// Name of the file containing the EBSD data
  std::string _filename;

  /// EBSD data file mesh information
  EBSDMeshGenerator::Geometry _geometry;
};
