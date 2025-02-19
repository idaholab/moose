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
 * Base class for generating fuel pins
 */
class DetailedPinMeshGeneratorBase : public MeshGenerator
{
public:
  DetailedPinMeshGeneratorBase(const InputParameters & parameters);

protected:
  void generatePin(std::unique_ptr<MeshBase> & mesh_base, const Point & center);

  /// unheated length of the fuel Pin at the entry of the assembly
  const Real _unheated_length_entry;
  /// heated length of the fuel Pin
  const Real _heated_length;
  /// unheated length of the fuel Pin at the exit of the assembly
  const Real _unheated_length_exit;
  /// axial location of nodes
  std::vector<Real> _z_grid;
  /// Distance between the neighbor fuel pins, pitch
  const Real _pitch;
  /// fuel Pin diameter
  const Real _pin_diameter;
  /// Number of cells in the axial direction
  const unsigned int _n_cells;
  /// Subdomain ID used for the mesh block
  const unsigned int & _block_id;
  /// Number of radial parts
  const unsigned int & _num_radial_parts;
  /// Counter for element numbering
  dof_id_type _elem_id;

public:
  static InputParameters validParams();
};
