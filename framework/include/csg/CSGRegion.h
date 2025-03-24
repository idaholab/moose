//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGSurface.h"

namespace CSG
{

/**
 * CSGRegions creates an internal representation of a CSG region, which can refer to
 * an intersection, union, complement, or halfspace
 * CSGHalfspace objects
 */
class CSGRegion
{
public:
  /// An enum for type of type of operation that defines region
  enum class RegionType
  {
    EMPTY,
    HALFSPACE,
    COMPLEMENT,
    INTERSECTION,
    UNION
  };

  /**
   * Default Constructor
   */
  CSGRegion();

  /**
   * Constructor for halfspace
   */
  CSGRegion(std::shared_ptr<CSGSurface> surf, const CSGSurface::Direction direction);

  /**
   * Constructor for union and intersection
   */
  CSGRegion(const CSGRegion & region_a,
            const CSGRegion & region_b,
            const CSGRegion::RegionType region_type);

  /**
   * Constructor for complement
   */
  CSGRegion(const CSGRegion & region, const CSGRegion::RegionType region_type);

  /**
   * Destructor
   */
  virtual ~CSGRegion() = default;

  /**
   * @brief gets the string representation of the region
   *
   * @return std::string
   */
  std::string toString() const { return _region_str; }

  /**
   * @brief Get the Region Type as a string
   *
   * @return const std::string
   */
  const std::string getRegionTypeString();

  /**
   * @brief Get the RegionType
   *
   * @return RegionType
   */
  RegionType getRegionType() const { return _region_type; }

  /**
   * @brief Get the list of surfaces associated with the region
   *
   * @return std::vector<std::shared_ptr<CSGSurface>>
   */
  std::vector<std::shared_ptr<CSGSurface>> getSurfaces() const { return _surfaces; };

protected:
  /// String representation of region - default empty string
  std::string _region_str;

  /// type of region - default empty
  RegionType _region_type;

  /// Surface list associated with the region
  std::vector<std::shared_ptr<CSGSurface>> _surfaces;
};

/**
 * @brief strip the leading and trailing parentheses fromt the string
 * if only the specified operator is present in the string
 *
 * @param region_str
 * @param op
 * @return std::string region string with () removed if applicable
 */
std::string stripRegionString(std::string region_str, std::string op);

/// Operation overloads for operation based region construction

// positve halfspace
const CSGRegion operator+(std::shared_ptr<CSGSurface> surf);

// negative halfspace
const CSGRegion operator-(std::shared_ptr<CSGSurface> surf);

// intersection
const CSGRegion operator&(const CSGRegion & region_a, const CSGRegion & region_b);

// union
const CSGRegion operator|(const CSGRegion & region_a, const CSGRegion & region_b);

// complement
const CSGRegion operator~(const CSGRegion & region);

} // namespace CSG
