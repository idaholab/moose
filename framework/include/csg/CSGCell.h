//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGHalfspace.h"
#include "CSGIntersection.h"

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
    material,
    empty,
    invalid
  };

  /**
   * Default constructor
   */
  CSGCell(const std::string name, const FillType fill_type);

  /**
   * Destructor
   */
  virtual ~CSGCell() = default;

  FillType getFillType() const { return _fill_type; }

  const std::string getName() const { return _name; }

  void addRegionHalfspace(const CSGHalfspace & halfspace) { _region.addNode(halfspace); }

  std::string getRegionAsString() const { return _region.toString(); }

protected:
  /// Name of surface
  const std::string _name;

  /// Cell region, represented as a CSGIntersection.
  /// We can generalize this further in the future, but this representation should
  /// be sufficient for now.
  CSGIntersection _region;

  /// Fill type of cell
  FillType _fill_type;
};
} // namespace CSG
