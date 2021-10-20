//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

#include <array>

/**
 * Mesh generated from parameters read from a DREAM3D EBSD file
 */
class EBSDMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  EBSDMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  struct Geometry
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
  const Geometry & getEBSDGeometry() const { return _geometry; }
  const std::string & getEBSDFilename() const { return _filename; }

protected:
  std::unique_ptr<MeshBase> & buildMeshSubgenerator();

  /// Read the EBSD data file header
  void readEBSDHeader();

  /// are we working on a distributed mesh?
  const bool _distributed;

  /// Name of the file containing the EBSD data
  const FileName & _filename;

  /// EBSD data file mesh information
  Geometry _geometry;

  // Number of coarsening levels available in adaptive mesh refinement
  const unsigned int _pre_refine;

  // Sub-MeshGenerator for the regular base mesh
  std::unique_ptr<MeshBase> & _base;
};
