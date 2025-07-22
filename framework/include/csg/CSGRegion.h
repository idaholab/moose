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
 * an intersection, union, complement, or half-space
 */
class CSGRegion
{
public:
  /**
   * Default Constructor
   */
  CSGRegion();

  /**
   * @brief Constructor for half-space of a surface
   *
   * @param surf referance to surface used to define the half-space
   * @param halfspace half-space to apply to surface (POSITIVE or NEGATIVE)
   */
  CSGRegion(const CSGSurface & surf, const CSGSurface::Halfspace halfspace);

  /**
   * @brief Constructor for union and intersection
   *
   * @param region_a reference to first region to union or intersect
   * @param region_b reference to second region to union or intersect
   * @param region_type type of region operation (UNION or INTERSECTION)
   */
  CSGRegion(const CSGRegion & region_a, const CSGRegion & region_b, std::string region_type);

  /**
   * @brief Constructor for complement or empty region (clear the region)
   *
   * @param region reference to region to apply complement
   * @param region_type type of region to apply (COMPLEMENT or EMPTY)
   */
  CSGRegion(const CSGRegion & region, std::string region_type);

  /**
   * Destructor
   */
  virtual ~CSGRegion() = default;

  /**
   * @brief gets the string representation of the region
   *
   * @return string representation of the region
   */
  const std::string & toString() const { return _region_str; }

  /**
   * @brief Get the region type
   *
   * @return region type
   */
  const std::string getRegionType() const { return _region_type; }

  /**
   * @brief Get the list of surfaces associated with the region
   *
   * @return list of pointers to surfaces that define the region
   */
  const std::vector<std::reference_wrapper<const CSGSurface>> & getSurfaces() const
  {
    return _surfaces;
  }

  /// Operator overload for &= add an intersection to the current region
  CSGRegion & operator&=(const CSGRegion & other_region);

  /// Operator overload for |= add a union to the current region
  CSGRegion & operator|=(const CSGRegion & other_region);

  /// Operator overload for checking if two CSGRegion objects are equal
  bool operator==(const CSGRegion & other) const;

  /// Operator overload for checking if two CSGRegion objects are not equal
  bool operator!=(const CSGRegion & other) const;

protected:
  /// String representation of region - defaults to empty string
  std::string _region_str;

  /// An enum for type of type of operation that defines region
  MooseEnum _region_type{"EMPTY HALFSPACE COMPLEMENT INTERSECTION UNION"};

  /// Surface list associated with the region
  std::vector<std::reference_wrapper<const CSGSurface>> _surfaces;
};

/**
 * @brief strip the leading and trailing parentheses from the string
 * if only the specified operator is present in the string
 *
 * @param region_str region string representation to simplify
 * @param op operator to consider
 * @return region string with () removed if applicable
 */
const std::string stripRegionString(std::string region_str, std::string op);

/// Operation overloads for operation based region construction

// positive half-space (+)
const CSGRegion operator+(const CSGSurface & surf);

// negative half-space (-)
const CSGRegion operator-(const CSGSurface & surf);

// intersection (&)
const CSGRegion operator&(const CSGRegion & region_a, const CSGRegion & region_b);

// union (|)
const CSGRegion operator|(const CSGRegion & region_a, const CSGRegion & region_b);

// complement (~)
const CSGRegion operator~(const CSGRegion & region);

} // namespace CSG
