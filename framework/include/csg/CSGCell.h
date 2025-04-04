//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGRegion.h"

namespace CSG
{

/**
 * CSGCell creates an internal representation of a Constructive Solid Geometry (CSG)
 * cell, which represents a region of space filled by a material or void
 */
class CSGCell
{
public:
  /// An enum for type of fill for cell region
  // TODO add support for universe and lattice fill
  enum class FillType
  {
    VOID,
    MATERIAL,
    UNIVERSE,
  };

  /**
   * Default constructor
   */
  CSGCell(const std::string name, const FillType fill_type, const CSGRegion & region);

  /**
   * Constructor for void cell
   */
  CSGCell(const std::string name, const CSGRegion & region);

  /**
   * Constructor for Material Cell; TODO: mat_name will be replaced with material object when
   * available
   */
  CSGCell(const std::string name, const std::string mat_name, const CSGRegion & region);

  /**
   * Destructor
   */
  virtual ~CSGCell() = default;

  FillType getFillType() const { return _fill_type; }

  template <typename T>
  T getFill();

  const std::string getName() const { return _name; }

  void updateRegion(const CSGRegion & region) { _region = region; }

  const CSGRegion & getRegion() const { return _region; }

  std::string getRegionAsString() const { return _region.toString(); }

  const std::string getFillTypeString();

protected:
  /// Name of surface
  const std::string _name;

  /// Fill type of cell
  FillType _fill_type;

  /// Fill
  const std::string _fill_material; // TODO: make this a material object
  const std::string _fill_universe; // TODO: make this a universe object

  /// Cell region, represented as a CSGRegion object
  CSGRegion _region;
};
} // namespace CSG
