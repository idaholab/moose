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
  _region_type = "EMPTY";
}

// halfspace constructor
CSGRegion::CSGRegion(const CSGSurface & surf, const CSGSurface::Halfspace halfspace)
{
  _region_type = "HALFSPACE";
  _region_str = ((halfspace == CSGSurface::Halfspace::POSITIVE) ? "+" : "-") + surf.getName();
  _surfaces.push_back(surf);
}

// intersection and union constructor
CSGRegion::CSGRegion(const CSGRegion & region_a,
                     const CSGRegion & region_b,
                     const std::string & region_type)
{
  _region_type = region_type;
  if (getRegionType() != RegionType::INTERSECTION && getRegionType() != RegionType::UNION)
    mooseError("Region type " + getRegionTypeString() + " is not supported for two regions.");
  if (region_a.getRegionType() == RegionType::EMPTY ||
      region_b.getRegionType() == RegionType::EMPTY)
    mooseError("Region operation " + getRegionTypeString() +
               " cannot be performed on an empty region.");

  std::string op = (getRegionType() == RegionType::UNION) ? " | " : " & ";
  auto a_string = stripRegionString(region_a.toString(), op);
  auto b_string = stripRegionString(region_b.toString(), op);

  _region_str = "(" + a_string + op + b_string + ")";
  const auto & a_surfs = region_a.getSurfaces();
  const auto & b_surfs = region_b.getSurfaces();
  _surfaces.insert(_surfaces.end(), a_surfs.begin(), a_surfs.end());
  _surfaces.insert(_surfaces.end(), b_surfs.begin(), b_surfs.end());
}

// complement or explicitly empty constructor
CSGRegion::CSGRegion(const CSGRegion & region, const std::string & region_type)
{
  _region_type = region_type;
  if (getRegionType() != RegionType::COMPLEMENT && getRegionType() != RegionType::EMPTY)
    mooseError("Region type " + getRegionTypeString() + " is not supported for a single region.");

  if (getRegionType() == RegionType::COMPLEMENT)
  {
    // no change to surfaces, but update string
    if (region.toString()[0] == '(')
      _region_str = "~" + region.toString();
    else
      _region_str = "~(" + region.toString() + ")";
    _surfaces = region.getSurfaces();
  }
  else if (getRegionType() == RegionType::EMPTY)
  {
    // reset the region and make it empty
    _region_str = "";
    _surfaces.clear();
  }
}

CSGRegion &
CSGRegion::operator&=(const CSGRegion & other_region)
{
  if (this != &other_region)
    *this = CSGRegion(*this, other_region, "INTERSECTION");
  return *this;
}

CSGRegion &
CSGRegion::operator|=(const CSGRegion & other_region)
{
  if (this != &other_region)
    *this = CSGRegion(*this, other_region, "UNION");
  return *this;
}

const std::string
stripRegionString(std::string region_str, std::string op)
{
  // add expected spacing around operator if not provided
  if (op == "|")
    op = " | ";
  if (op == "&")
    op = " & ";

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

// positive halfspace
const CSGRegion
operator+(const CSGSurface & surf)
{
  return CSGRegion(surf, CSGSurface::Halfspace::POSITIVE);
}

// negative halfspace
const CSGRegion
operator-(const CSGSurface & surf)
{
  return CSGRegion(surf, CSGSurface::Halfspace::NEGATIVE);
}

// intersection
const CSGRegion
operator&(const CSGRegion & region_a, const CSGRegion & region_b)
{
  return CSGRegion(region_a, region_b, "INTERSECTION");
}

// union
const CSGRegion
operator|(const CSGRegion & region_a, const CSGRegion & region_b)
{
  return CSGRegion(region_a, region_b, "UNION");
}

// complement
const CSGRegion
operator~(const CSGRegion & region)
{
  return CSGRegion(region, "COMPLEMENT");
}

bool
CSGRegion::operator==(const CSGRegion & other) const
{
  const bool region_type_eq = this->getRegionType() == other.getRegionType();
  const bool region_str_eq = this->toString() == other.toString();
  if (region_type_eq && region_str_eq)
  {
    const auto & all_surfs = getSurfaces();
    const auto & other_surfs = other.getSurfaces();
    const bool num_cells_eq = all_surfs.size() == other_surfs.size();
    if (num_cells_eq)
    {
      for (const auto i : index_range(all_surfs))
        if (all_surfs[i].get() != other_surfs[i].get())
          return false;
      return true;
    }
    else
      return false;
  }
  else
    return false;
}

bool
CSGRegion::operator!=(const CSGRegion & other) const
{
  return !(*this == other);
}

} // namespace CSG
