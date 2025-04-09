//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGRegion.h"

namespace CSG
{

CSGRegion::CSGRegion()
{
  _region_str = "";
  _region_type = CSGRegion::RegionType::EMPTY;
};

// halfspace constructor
CSGRegion::CSGRegion(std::shared_ptr<CSGSurface> surf, const CSGSurface::Direction direction)
  : _region_type(CSGRegion::RegionType::HALFSPACE)
{
  _region_str = ((direction == CSGSurface::Direction::positive) ? "+" : "-") + surf->getName();
  _surfaces.push_back(surf);
}

// intersection and union constructor
CSGRegion::CSGRegion(const CSGRegion & region_a,
                     const CSGRegion & region_b,
                     const CSGRegion::RegionType region_type)
  : _region_type(region_type)
{
  if (_region_type != CSGRegion::RegionType::INTERSECTION &&
      _region_type != CSGRegion::RegionType::UNION)
    mooseError("Region type " + getRegionTypeString() + " is not supported for two regions.");
  else if (region_a.getRegionType() == CSGRegion::RegionType::EMPTY ||
           region_b.getRegionType() == CSGRegion::RegionType::EMPTY)
    mooseError("Region operation " + getRegionTypeString() +
               " cannot be performed on an empty region.");
  else
  {
    std::string op = (_region_type == CSGRegion::RegionType::UNION) ? " | " : " & ";
    auto a_string = stripRegionString(region_a.toString(), op);
    auto b_string = stripRegionString(region_b.toString(), op);

    _region_str = "(" + a_string + op + b_string + ")";
    auto a_surfs = region_a.getSurfaces();
    auto b_surfs = region_b.getSurfaces();
    _surfaces.insert(_surfaces.end(), a_surfs.begin(), a_surfs.end());
    _surfaces.insert(_surfaces.end(), b_surfs.begin(), b_surfs.end());
  }
}

// complement or explicitly empty constructor
CSGRegion::CSGRegion(const CSGRegion & region, const CSGRegion::RegionType region_type)
  : _region_type(region_type)
{
  if (_region_type != CSGRegion::RegionType::COMPLEMENT &&
      _region_type != CSGRegion::RegionType::EMPTY)
    mooseError("Region type " + getRegionTypeString() + " is not supported for a single region");

  if (_region_type == CSGRegion::RegionType::COMPLEMENT)
  {
    // no change to surfaces, but update string
    _region_str = "~" + region.toString();
    _surfaces = region.getSurfaces();
  }
  else if (_region_type == CSGRegion::RegionType::EMPTY)
  {
    // reset the region and make it empty
    _region_str = "";
    _surfaces.clear();
  }
}

const std::string
CSGRegion::getRegionTypeString()
{
  switch (_region_type)
  {
    case RegionType::EMPTY:
      return "EMPTY";
    case RegionType::HALFSPACE:
      return "HALFSPACE";
    case RegionType::COMPLEMENT:
      return "COMPLEMENT";
    case RegionType::INTERSECTION:
      return "INTERSECTION";
    case RegionType::UNION:
      return "UNION";
    default:
      return "INVALID";
  }
}

CSGRegion &
CSGRegion::operator&=(const CSGRegion & other_region)
{
  if (this != &other_region)
    *this = CSGRegion(*this, other_region, CSGRegion::RegionType::INTERSECTION);
  return *this;
}

CSGRegion &
CSGRegion::operator|=(const CSGRegion & other_region)
{
  if (this != &other_region)
    *this = CSGRegion(*this, other_region, CSGRegion::RegionType::UNION);
  return *this;
}

std::string
stripRegionString(std::string region_str, std::string op)
{
  std::vector<std::string> ops_list = {" | ", " & ", "~"};
  if (op.compare(" | ") && op.compare(" & "))
  { // compare() returns non zero if strings are not equal
    mooseError(
        "Region string can only be simplified based on intersection (&) and union (|) operations.");
  }

  // find if there are any operators in string already that are not the op of interest
  // if not, then remove parentheses:
  bool remove_par = true; // assume parentheses can be removed unless otherwise
  for (auto opi : ops_list)
    if (opi != op)
      // only look for strings that are not of the current op type
      if (region_str.find(opi) != std::string::npos)
      {
        remove_par = false;
        break;
      }

  if (remove_par)
    region_str.erase(std::remove_if(region_str.begin(),
                                    region_str.end(),
                                    [](char c) { return c == '(' || c == ')'; }),
                     region_str.end());
  return region_str;
}

// Operators for region construction

// positve halfspace
const CSGRegion
operator+(std::shared_ptr<CSGSurface> surf)
{
  return CSGRegion(surf, CSGSurface::Direction::positive);
}

// negative halfspace
const CSGRegion
operator-(std::shared_ptr<CSGSurface> surf)
{
  return CSGRegion(surf, CSGSurface::Direction::negative);
}

// intersection
const CSGRegion
operator&(const CSGRegion & region_a, const CSGRegion & region_b)
{
  return CSGRegion(region_a, region_b, CSGRegion::RegionType::INTERSECTION);
}

// union
const CSGRegion
operator|(const CSGRegion & region_a, const CSGRegion & region_b)
{
  return CSGRegion(region_a, region_b, CSGRegion::RegionType::UNION);
}

// complement
const CSGRegion
operator~(const CSGRegion & region)
{
  return CSGRegion(region, CSGRegion::RegionType::COMPLEMENT);
}

} // namespace CSG
