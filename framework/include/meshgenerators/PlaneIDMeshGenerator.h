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

/**
 * Assigns plane extra ID for existing 3D meshes
 */
class PlaneIDMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  PlaneIDMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// get plane ID for given plane coordiantes
  int getPlaneID(const Point & p) const;
  /// input mesh for adding reporting ID
  std::unique_ptr<MeshBase> & _input;
  /// coordinates of planes
  std::vector<Real> _planes;
  /// index of plane axis
  const unsigned int _axis_index;
  /// name of integer ID
  const std::string _element_id_name;
};
