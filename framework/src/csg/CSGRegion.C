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

char
CSGRegion::regionSymbol(const RegionType region_type)
{
  mooseAssert(region_type == RegionType::COMPLEMENT || region_type == RegionType::UNION ||
                  region_type == RegionType::INTERSECTION,
              "Unexpected region type");

  constexpr std::array<char, 5> symbols = {
      '\0', // CSGRegion::RegionType::EMPTY (unused)
      '\0', // CSGRegion::RegionType::HALFSPACE (unused)
      '~',  // CSGRegion::RegionType::COMPLEMENT
      '&',  // CSGRegion::RegionType::INTERSECTION
      '|'   // CSGRegion::RegionType::UNION
  };
  static_assert(symbols[static_cast<std::size_t>(RegionType::COMPLEMENT)] == '~');
  static_assert(symbols[static_cast<std::size_t>(RegionType::INTERSECTION)] == '&');
  static_assert(symbols[static_cast<std::size_t>(RegionType::UNION)] == '|');

  return symbols[static_cast<std::size_t>(region_type)];
}

char
CSGRegion::halfspaceSymbol(const CSGSurface::Halfspace halfspace)
{
  mooseAssert(halfspace == CSGSurface::Halfspace::POSITIVE ||
                  halfspace == CSGSurface::Halfspace::NEGATIVE,
              "Unexpected halfspace");

  constexpr std::array<char, 2> symbols = {
      '+', // CSGSurface::Halfspace::POSITIVE
      '-'  // CSGSurface::Halfspace::NEGATIVE
  };
  static_assert(symbols[static_cast<std::size_t>(CSGSurface::Halfspace::POSITIVE)] == '+');
  static_assert(symbols[static_cast<std::size_t>(CSGSurface::Halfspace::NEGATIVE)] == '-');

  return symbols[static_cast<std::size_t>(halfspace)];
}

CSGRegion::CSGRegion()
{
  _region_type = "EMPTY";
  _surfaces.clear();
  _postfix_tokens.clear();
}

// halfspace constructor
CSGRegion::CSGRegion(const CSGSurface & surf, const CSGSurface::Halfspace halfspace)
{
  _region_type = "HALFSPACE";
  _surfaces.push_back(surf);

  // (halfspace surf) in postfix is represented as (surf halfspace)
  _postfix_tokens.push_back(surf);
  _postfix_tokens.push_back(halfspace);
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

  // (region_a region_type region_b) in postfix is represented as (region_a region_b region_type)
  _postfix_tokens.insert(_postfix_tokens.end(),
                         region_a.getPostfixTokens().begin(),
                         region_a.getPostfixTokens().end());
  _postfix_tokens.insert(_postfix_tokens.end(),
                         region_b.getPostfixTokens().begin(),
                         region_b.getPostfixTokens().end());
  _postfix_tokens.push_back(getRegionType());
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
    _surfaces = region.getSurfaces();
    // (complement region) in postfix is represented as (region complement)
    _postfix_tokens = region.getPostfixTokens();
    _postfix_tokens.push_back(getRegionType());
  }
  else if (getRegionType() == RegionType::EMPTY)
  {
    _surfaces.clear();
    _postfix_tokens.clear();
  }
}

nlohmann::json
CSGRegion::toInfixJSON() const
{
  // Return an empty JSON object if no postfix tokens are defined
  if (_postfix_tokens.empty())
    return nlohmann::json::parse("[]");

  // Build the region string using a stack, iterating through each token within _postfix_tokens
  std::stack<std::string> postfix_stack;
  for (auto i : index_range(_postfix_tokens))
  {
    const auto & token = _postfix_tokens[i];
    // Surface: Push name to stack
    if (const auto surface_ref_ptr = std::get_if<std::reference_wrapper<const CSGSurface>>(&token))
      postfix_stack.push(surface_ref_ptr->get().getName());
    // Halfspaces and region operators
    else
    {
      std::string region_string;
      // Halfspace: Pop from the stack, update region string, push back
      if (const auto halfspace_ptr = std::get_if<CSGSurface::Halfspace>(&token))
      {
        std::string symbol = std::string(1, halfspaceSymbol(*halfspace_ptr));
        region_string = "\"" + symbol + postfix_stack.top() + "\"";
        postfix_stack.pop();
      }
      // Region operator: Pop 1 or 2 values, update region string, push back
      else
      {
        const auto region = std::get<RegionType>(token);
        const std::string symbol{regionSymbol(region)};
        if (region == RegionType::COMPLEMENT)
        {
          region_string = postfix_stack.top();
          postfix_stack.pop();
          if (region_string[0] == '[')
            region_string = "\"" + symbol + "\", " + region_string;
          else
            region_string = "\"" + symbol + "\", [" + region_string + "]";
        }
        else
        {
          auto region_string_b = postfix_stack.top();
          postfix_stack.pop();
          auto region_string_a = postfix_stack.top();
          postfix_stack.pop();
          region_string = region_string_a + ", \"" + symbol + "\", " + region_string_b;
          // Skip putting parentheses around the region string if the next region operator in the
          // postfix token list is identical
          if (!nextRegionOpIsIdentical(region, i + 1))
            region_string = "[" + region_string + "]";
        }
      }
      postfix_stack.push(region_string);
    }
  }

  // Top of stack should now have region string we desire. Now, we
  // parse the string into a JSON object
  std::string region_string = postfix_stack.top();
  // Wrap region string in square brackets so that it is always treated as a
  // list in the output JSON object
  if (region_string[0] != '[')
    region_string = "[" + region_string + "]";
  return nlohmann::json::parse(region_string);
}

std::vector<std::string>
CSGRegion::toPostfixStringList() const
{
  std::vector<std::string> postfix_string_list;
  postfix_string_list.reserve(_postfix_tokens.size());
  for (const auto & token : _postfix_tokens)
    postfix_string_list.push_back(postfixTokenToString(token));
  return postfix_string_list;
}

std::string
CSGRegion::postfixTokenToString(const PostfixTokenVariant & token) const
{
  // Lambda function to return all variant types as strings
  return std::visit(
      [](auto && arg) -> std::string
      {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::reference_wrapper<const CSGSurface>>)
          return arg.get().getName();
        else if constexpr (std::is_same_v<T, RegionType>)
          return std::string{regionSymbol(arg)};
        else // if constexpr (std::is_same_v<T, CSGSurface::Halfspace>)
          return std::string{halfspaceSymbol(arg)};
      },
      token);
}

bool
CSGRegion::nextRegionOpIsIdentical(const RegionType region,
                                   const std::size_t postfix_token_index) const
{
  for (const auto i : make_range(postfix_token_index, _postfix_tokens.size()))
    if (const auto region_ptr = std::get_if<RegionType>(&_postfix_tokens[i]))
      return region == *region_ptr;
  return false;
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
  const bool region_eq = this->toPostfixStringList() == other.toPostfixStringList();
  if (region_type_eq && region_eq)
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
