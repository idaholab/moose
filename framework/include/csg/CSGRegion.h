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
#include "CSGTransformation.h"
#include "MooseEnum.h"

namespace CSG
{

/**
 * CSGRegions creates an internal representation of a CSG region, which can refer to
 * an intersection, union, complement, or half-space
 */
class CSGRegion
{
public:
  /// Enum for representing region types, defined to match _region_type MooseEnum
  enum class RegionType
  {
    EMPTY,
    HALFSPACE,
    COMPLEMENT,
    INTERSECTION,
    UNION
  };

  /**
   * Type definition for a variant that represents the datatypes for entries within the list that
   * represents the region in postfix notation. This can be a surface reference, a region
   * type, or halfspace
   */
  typedef std::variant<std::reference_wrapper<const CSGSurface>, RegionType, CSGSurface::Halfspace>
      PostfixTokenVariant;

  /**
   * @return The symbol associated with the given region type
   */
  static char regionSymbol(const RegionType region_type);
  /**
   * @return The symbol associated with the given halfspace
   */
  static char halfspaceSymbol(const CSGSurface::Halfspace halfspace);

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
  CSGRegion(const CSGRegion & region_a,
            const CSGRegion & region_b,
            const std::string & region_type);

  /**
   * @brief Constructor for complement or empty region (clear the region)
   *
   * @param region reference to region to apply complement
   * @param region_type type of region to apply (COMPLEMENT or EMPTY)
   */
  CSGRegion(const CSGRegion & region, const std::string & region_type);

  /**
   * Destructor
   */
  virtual ~CSGRegion() = default;

  /**
   * @brief gets the infix JSON representation of the region, which involves converting
   *        region representation from postfix to infix notation
   *
   * @return infix JSON representation of the region
   */
  nlohmann::json toInfixJSON() const;

  /**
   * @brief gets the list of postfix tokens of the region in string representation
   *
   * @return list of postfix tokens that represent the region
   */
  std::vector<std::string> toPostfixStringList() const;

  /**
   * @brief converts postfix token from PostfixTokenVariant to string representation
   *
   * @param token postfix token of type PostfixTokenVariant
   * @return string representation of postfix token
   */
  std::string postfixTokenToString(const PostfixTokenVariant & token) const;

  /**
   * @brief Get the region type
   *
   * @return region type enum
   */
  RegionType getRegionType() const { return _region_type.getEnum<RegionType>(); }

  /**
   * @brief Get the region type as a string
   *
   * @return region type string
   */
  const std::string getRegionTypeString() const { return _region_type; }

  /**
   * @brief Get the list of surfaces associated with the region
   *
   * @return list of pointers to surfaces that define the region
   */
  const std::vector<std::reference_wrapper<const CSGSurface>> & getSurfaces() const
  {
    return _surfaces;
  }

  /// Operator overload for &= which creates an intersection between the current region and the other_region
  CSGRegion & operator&=(const CSGRegion & other_region);

  /// Operator overload for |= which creates a union of the current region with the other_region
  CSGRegion & operator|=(const CSGRegion & other_region);

  /// Operator overload for checking if two CSGRegion objects are equal
  bool operator==(const CSGRegion & other) const;

  /// Operator overload for checking if two CSGRegion objects are not equal
  bool operator!=(const CSGRegion & other) const;

protected:
  /**
   * @brief Get the list of postfix tokens associated with the region
   *
   * @return list of tokens that define the region in postfix notation
   */
  const std::vector<PostfixTokenVariant> & getPostfixTokens() const { return _postfix_tokens; }

  /**
   * @brief Iterate through postfix tokens and check if next region operator matches the given
   * operator
   *
   * @param region the region type
   * @param postfix_token_index index in _postfix_tokens to start  region operator comparisons
   * @return true if next region operator in _postfix_tokens matches region_op_string, false
   * otherwise
   */
  bool nextRegionOpIsIdentical(const RegionType region,
                               const std::size_t postfix_token_index) const;

  /// String representation of region - defaults to empty string
  std::string _region_str;

  /// An enum for type of type of operation that defines region
  MooseEnum _region_type{"EMPTY=0 HALFSPACE=1 COMPLEMENT=2 INTERSECTION=3 UNION=4"};

  /// Surface list associated with the region
  std::vector<std::reference_wrapper<const CSGSurface>> _surfaces;

  /// List of tokens representing the region in postfix notation
  std::vector<PostfixTokenVariant> _postfix_tokens;
};

/// Operation overloads for operation based region construction

/// Overload for creating a region from the positive half-space (+) of a surface
const CSGRegion operator+(const CSGSurface & surf);

/// Overload for creating a region from the negative half-space (-) of a surface
const CSGRegion operator-(const CSGSurface & surf);

/// Overload for creating a region from the the intersection (&) of two regions
const CSGRegion operator&(const CSGRegion & region_a, const CSGRegion & region_b);

/// Overload for creating a region from the union (|) of two regions
const CSGRegion operator|(const CSGRegion & region_a, const CSGRegion & region_b);

/// Overload for creating a region from the complement (~) of another region
const CSGRegion operator~(const CSGRegion & region);

} // namespace CSG
