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
#include "MooseEnum.h"

/**
 * A class to store mesh information that is globally applicable to a reactor.
 */
class ReactorMeshParams : public MeshGenerator
{
public:
  static InputParameters validParams();

  ReactorMeshParams(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  void generateData() override{};

protected:
  /// The number of dimension in the mesh
  const MooseEnum _dim;

  /// The geometry type for the reactor
  const MooseEnum _geom;

  /// The the flat-to-flat size of assemblies in the reactor.
  const Real _assembly_pitch;

  ///The heights of the axial regions.
  std::vector<Real> _axial_regions;

  ///The number of mesh divisions in each axial region.
  std::vector<unsigned int> _axial_mesh_intervals;

  ///Boundary id assigned to top boundary of extruded mesh.
  boundary_id_type _top_boundary;

  ///Boundary id assigned to bottom boundary of extruded mesh.
  boundary_id_type _bottom_boundary;

  ///Boundary id assigned to outer radial boundary of core mesh.
  boundary_id_type _radial_boundary;
};
