//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGUniverseEngUnit.h"
#include "CSGBase.h"

namespace CSG
{

/**
 * PinUniverseEngUnit is a CSGUniverseEngUnit that represents a 2D pin universe
 * (concentric cylinders surrounded by an outer region) using basic
 * engineering-scale attrbutes, including ring radii and fill material names for each radial region
 * of the pin structure.
 *
 * Implements:
 *   - expandUnit(): creates a universe that represents the pin cell structure
 *   - clone(): returns a deep copy of the pin engineering unit
 *   - getAttributes(): returns ring_radii, fill_mats, and geometry_type
 */
class PinUniverseEngUnit : public CSGUniverseEngUnit
{
public:
  /**
   * @brief Constructor for PinUniverseEngUnit
   *
   * @param name unique name of the unit
   * @param ring_radii list of ring radii of cylindrical pin regions, which need to be in ascending
   *                   order
   * @param fill_mats Vector of material names that represent the fill of each radial region
   *
   * @note The number of fill material names should be one larger than the number of ring radii in
   * the pin. The extra material name represents the material fill that surrounds the region outside
   * of the outermost ring.
   *
   */
  PinUniverseEngUnit(const std::string & name,
                     const std::vector<Real> & ring_radii,
                     const std::vector<std::string> & fill_mats);
  /**
   * @brief Return the pin engeering unit attributes for this object.
   *
   * @return map containing: ring_radii (std::vector<Real>) and fill_mats (std::vector<std::string>)
   */
  std::unordered_map<std::string, AttributeVariant> getAttributes() const override;

protected:
  /**
   * @brief Return a deep copy of this unit.
   *
   * @return unique_ptr to a new PinUniverseEngUnit with identical parameters
   */
  std::unique_ptr<CSGUniverseEngUnit> clone() const override;

  /**
   * @brief Represent the pin cell as a single universe that contains cells that define each region
   * of the pin cell structure
   */
  void expandUnit() override;

private:
  /// List of ring radii of pin cell
  const std::vector<Real> _ring_radii;

  /// Region IDs of pin cell
  std::vector<std::string> _fill_mats;
};

} // namespace CSG
