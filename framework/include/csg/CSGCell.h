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

class CSGUniverse;

/**
 * CSGCell creates an internal representation of a Constructive Solid Geometry (CSG)
 * cell, which represents a region of space filled by a material or void
 */
class CSGCell
{
public:
  /**
   * Constructor for void cell
   *
   * @param name name of cell
   * @param region cell region
   */
  CSGCell(const std::string & name, const CSGRegion & region);

  /**
   * Constructor for Material Cell
   *
   * @param name name of cell
   * @param mat_name name of material to use as the cell fill
   * @param region cell region
   */
  CSGCell(const std::string & name, const std::string & mat_name, const CSGRegion & region);

  /**
   * Constructor for Universe Cell
   *
   * @param name name of cell
   * @param univ universe to be the fill
   * @param region cell region
   */
  CSGCell(const std::string & name, const CSGUniverse * univ, const CSGRegion & region);

  /**
   * Destructor
   */
  virtual ~CSGCell() = default;

  /**
   * @brief Get the type of fill for the cell
   *
   * @return fill type
   */
  const std::string getFillType() const { return _fill_type; }

  /**
   * @brief Get the cell fill if fill type is UNIVERSE
   *
   * @return Reference to CSGUniverse fill
   */
  const CSGUniverse & getFillUniverse() const;

  /**
   * @brief Get the cell fill material name if fill fype is CSG_MATERIAL
   *
   * @return const std::string
   */
  const std::string & getFillMaterial() const;

  /**
   * @brief Get the name of the fill
   *
   * @return std::string fill name
   */
  const std::string & getFillName() const { return _fill_name; }

  /**
   * @brief Get the cell name
   *
   * @return const std::string cell name
   */
  const std::string & getName() const { return _name; }

  /**
   * @brief Get the cell region
   *
   * @return const CSGRegion& region of the cell
   */
  const CSGRegion & getRegion() const { return _region; }

  /**
   * @brief Get the string representation of the cell region
   *
   * @return std::string string representation of the cell region
   */
  const std::string & getRegionAsString() const { return _region.toString(); }

  /// Operator overload for checking if two CSGCell objects are equal
  bool operator==(const CSGCell & other) const;

  /// Operator overload for checking if two CSGCell objects are not equal
  bool operator!=(const CSGCell & other) const;

protected:
  // set the name of the cell - intentionally not public because
  // name needs to be managed at the CSGCellList level
  void setName(const std::string & name) { _name = name; }

  // update the region of the cell to a new region - not public because
  // it needs to be called from CSGBase so that the surfaces can be checked first.
  void updateRegion(const CSGRegion & region) { _region = region; }

  /// Name of surface
  std::string _name;

  /// An enum for type of fill for cell region
  // TODO: add support for lattice fill
  MooseEnum _fill_type{"VOID CSG_MATERIAL UNIVERSE"};

  /// name of the fill object
  std::string _fill_name;

  /// Cell region, represented as a CSGRegion object
  CSGRegion _region;

  /// Fill object if fill is CSGUniverse
  const CSGUniverse * _fill_universe;

  friend class CSGCellList; // needed for setName() access
  friend class CSGBase;     // needed for updateRegion() access
};
} // namespace CSG
