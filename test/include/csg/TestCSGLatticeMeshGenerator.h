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

class TestCSGLatticeMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  TestCSGLatticeMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  void generateData() override {};

  std::unique_ptr<CSG::CSGBase> generateCSG() override;

protected:
  /// hexagonal or cartesian lattice type
  std::string _lattice_type;
  /// pitch of the lattice
  Real _pitch;
  const std::vector<MeshGeneratorName> _input_names;
  /// Pointers to the input mesh
  std::vector<std::unique_ptr<MeshBase> *> _mesh_ptrs;
  /// Holds the input CSGBase objects
  std::vector<std::unique_ptr<CSG::CSGBase> *> _input_csgs;
  /// input pattern of universes
  std::vector<std::vector<unsigned int>> _pattern;
};
