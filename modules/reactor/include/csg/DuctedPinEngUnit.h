//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGCellEngUnit.h"
#include "CSGBase.h"

namespace CSG
{

/**
 * DuctedPinEngUnit is a CSGCellEngUnit that represents a pin cell structure
 * (concentric cylinders surrounded by one or more ducted regions) using basic
 * engineering-scale attrbutes, including geometry type (hex or square), ring radii,
 * duct apothems, axial boundaries, and radial and axial region ID names for each region
 * of the pin structure. This engineering unit supports both 3D and 2D/infinite pin geometries.
 *
 * Implements:
 *   - expandUnit(): creates a universe that represents the pin cell structure
 *   - clone(): returns a deep copy of the pin engineering unit
 *   - getAttributes(): returns duct_apothems, ring_radii, region_ids, and geometry_type
 */
class DuctedPinEngUnit : public CSGCellEngUnit
{
public:
  /**
   * @brief Constructor for DuctedPinEngUnit
   *
   * @param name unique name of the unit
   * @param geometry_type geometry type of pin cell ("Hex" or "Square")
   * @param ring_radii list of ring radii of cylindrical pin regions, which need to be in ascending
   *                   order
   * @param duct_apothems list of duct apothems that surround cylindrical regions in pin, which need
   *                      to be in ascending order
   * @param region_ids 2-D vector of region IDs that represent each radial (cylindrical and ducted)
   *                   region and axial region
   * @param axial_boundaries list of axial boundaries of extruded pin
   */
  DuctedPinEngUnit(const std::string & name,
                   const std::string & geometry_type,
                   const std::vector<Real> & ring_radii,
                   const std::vector<Real> & duct_apothems,
                   const std::vector<std::vector<unsigned int>> & region_ids,
                   const std::vector<Real> & axial_boundaries);
  /**
   * @brief Return the pin engeering unit attributes for this object.
   *
   * @return map containing: geometry_type (std::string), ring_radii (std::vector<Real>),
   * duct_apothems (std::vector<Real>), region_ids (std::vector<std::vector<unsigned int>>),
   * and axial_boundaries (std::vector<Real>)
   */
  std::unordered_map<std::string, AttributeVariant> getAttributes() const override;

protected:
  /**
   * @brief Return a deep copy of this unit.
   *
   * @return unique_ptr to a new DuctedPinEngUnit with identical parameters
   */
  std::unique_ptr<CSGCellEngUnit> clone() const override;

  /**
   * @brief Represent the pin cell as a single universe that contains cells that define each region
   * of the pin cell structure
   */
  void expandUnit() override;

private:
  /// Geometry type of pin cell structure (hex or square)
  const std::string & _geometry_type;

  /// List of ring radii of pin cell
  const std::vector<Real> _ring_radii;

  /// List of duct apothems of pin cell
  const std::vector<Real> _duct_apothems;

  /// Region IDs of pin cell
  std::vector<std::vector<unsigned int>> _region_ids;

  /// Axial boundaries for an extruded pin cell
  std::vector<Real> _axial_boundaries;
};

} // namespace CSG
