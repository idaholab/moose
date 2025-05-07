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
   * Constructor for Universe Cell
   */
  CSGCell(const std::string name, const CSGUniverse & univ, const CSGRegion & region);

  /**
   * Destructor
   */
  virtual ~CSGCell() = default;

  /**
   * @brief Get the type of fill for the cell
   *
   * @return FillType
   */
  FillType getFillType() const { return _fill_type; }

  /**
   * @brief Get the string representation of the fill type
   *
   * @return const std::string
   */
  const std::string getFillTypeString();

  /**
   * @brief Get the cell fill if FillType is UNIVERSE
   *
   * @return CSGUniverse pointer
   */
  const std::shared_ptr<CSGUniverse> getFillUniverse();

  /**
   * @brief Get the cell fill material name if FillType is MATERIAL
   * //TODO: update to return material object when materials are implemented
   *
   * @return const std::string
   */
  const std::string getFillMaterial();

  /**
   * @brief Get the name of the fill
   *
   * @return std::string
   */
  std::string getFillName() const { return _fill_name; }

  /**
   * @brief Get the cell name
   *
   * @return const std::string
   */
  const std::string getName() const { return _name; }

  /**
   * @brief upate the region of the cell
   *
   * @param region
   */
  void updateRegion(const CSGRegion & region) { _region = region; }

  /**
   * @brief Get the cell region
   *
   * @return const CSGRegion&
   */
  const CSGRegion & getRegion() const { return _region; }

  /**
   * @brief Get the string representation of the cell region
   *
   * @return std::string
   */
  std::string getRegionAsString() const { return _region.toString(); }

protected:
  /// Name of surface
  const std::string _name;

  /// Fill type of cell
  FillType _fill_type;

  /// name of the fill object
  std::string _fill_name;

  /// Cell region, represented as a CSGRegion object
  CSGRegion _region;

  /// Fill object if fill is CSGUniverse
  const std::shared_ptr<CSGUniverse> _fill_universe;
};
} // namespace CSG
