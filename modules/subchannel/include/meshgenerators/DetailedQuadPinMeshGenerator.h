//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DetailedPinMeshGeneratorBase.h"

/**
 * Mesh generator for fuel pins in a quadrilateral lattice
 */
class DetailedQuadPinMeshGenerator : public DetailedPinMeshGeneratorBase
{
public:
  DetailedQuadPinMeshGenerator(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that comes from another generator
  std::unique_ptr<MeshBase> & _input;
  /// Number of subchannels in the x direction
  const unsigned int & _nx;
  /// Number of subchannels in the y direction
  const unsigned int & _ny;

public:
  static InputParameters validParams();
};
