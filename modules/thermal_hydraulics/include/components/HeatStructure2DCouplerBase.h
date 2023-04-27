//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryBase.h"
#include "MeshAlignment.h"
#include "HeatStructureBase.h"

/**
 * Couples boundaries of two 2D heat structures
 */
class HeatStructure2DCouplerBase : public BoundaryBase
{
public:
  HeatStructure2DCouplerBase(const InputParameters & parameters);

protected:
  virtual void setupMesh() override;
  virtual void init() override;
  virtual void check() const override;

  /// Primary and secondary heat structure names
  const std::vector<std::string> _hs_names;
  /// Primary and secondary heat structure boundaries
  const std::vector<BoundaryName> _hs_boundaries;

  /// Mesh alignment
  MeshAlignment _mesh_alignment;
  /// Flag for each heat structure being HeatStructurePlate
  std::vector<bool> _is_plate;
  /// Flag for each heat structure deriving from HeatStructureCylindricalBase
  std::vector<bool> _is_cylindrical;
  /// Heat structure side types for each boundary
  std::vector<Component2D::ExternalBoundaryType> _hs_side_types;
  /// Areas for the primary and secondary sides
  std::vector<Real> _areas;
  /// Area fractions by which to multiply coupling terms
  std::vector<Real> _coupling_area_fractions;

public:
  static InputParameters validParams();
};
