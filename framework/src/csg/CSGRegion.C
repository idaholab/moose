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

std::string
CSGRegion::toInfixString() const
{
  // Return an empty string if no postfix tokens are defined
  if (_postfix_tokens.empty())
    return "";

  // Define maps to conert halfspace and region operator names to their symbols
  std::map<std::string, std::string> halfspace_to_symbol = {{"halfspace::POSITIVE", "+"},
                                                            {"halfspace::NEGATIVE", "-"}};
  std::map<std::string, std::string> region_op_to_symbol = {{"regionOperator::INTERSECTION", "&"},
                                                            {"regionOperator::UNION", "|"},
                                                            {"regionOperator::COMPLEMENT", "~"}};

  // Define which region operators are unary. This will set how many pop operations are needed to
  // build the region string
  std::set<std::string> unary_region_ops = {"regionOperator::COMPLEMENT"};

  // Build the region string using a stack, iterating through each token within _postfix_tokens
  std::stack<std::string> postfix_stack;
  for (auto i : index_range(_postfix_tokens))
  {
    const auto & postfix_token_string = postfixTokenToString(_postfix_tokens[i]);
    if (postfix_token_string.find("::") == std::string::npos)
      // For surfaces, push the surface name to the stack
      postfix_stack.push(postfix_token_string);
    else
    {
      std::string region_string;
      if (postfix_token_string.find("halfspace::") != std::string::npos)
      {
        // For halfspaces, pop the value from the stack, update region string, and push back
        region_string = halfspace_to_symbol[postfix_token_string] + postfix_stack.top();
        postfix_stack.pop();
      }
      else
      {
        const auto & region_op_symbol = region_op_to_symbol[postfix_token_string];
        // For region operators, pop 1 or 2 values from the stack, update the region string, and
        // push back
        if (unary_region_ops.count(postfix_token_string))
        {
          region_string = postfix_stack.top();
          postfix_stack.pop();
          if (region_string[0] == '(')
            region_string = region_op_symbol + region_string;
          else
            region_string = region_op_symbol + "(" + region_string + ")";
        }
        else
        {
          auto region_string_b = postfix_stack.top();
          postfix_stack.pop();
          auto region_string_a = postfix_stack.top();
          postfix_stack.pop();
          region_string = region_string_a + " " + region_op_symbol + " " + region_string_b;
          // Skip putting parentheses around the region string if the next region operator in the
          // postfix token list is identical
          if (!nextRegionOpIsIdentical(postfix_token_string, i + 1))
            region_string = "(" + region_string + ")";
        }
      }
      postfix_stack.push(region_string);
    }
  }

  // Top of stack should now have region string we desire
  return postfix_stack.top();
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
        {
          // For surfaces, return the surface name
          const CSGSurface & surf = arg.get();
          return surf.getName();
        }
        else if constexpr (std::is_same_v<T, RegionType>)
        {
          // For region types, return the name of the region type prepended with "regionOperator::"
          RegionType region_type = arg;
          if (region_type == CSGRegion::RegionType::INTERSECTION)
            return "regionOperator::INTERSECTION";
          else if (region_type == CSGRegion::RegionType::UNION)
            return "regionOperator::UNION";
          else
            return "regionOperator::COMPLEMENT";
        }
        else // if constexpr (std::is_same_v<T, CSGSurface::Halfspace>)
        {
          // For halfspaces, return the direction of the halfspace prepended with "halfspace::"
          CSGSurface::Halfspace halfspace = arg;
          return std::string("halfspace::") +
                 std::string((halfspace == CSGSurface::Halfspace::POSITIVE) ? "POSITIVE"
                                                                            : "NEGATIVE");
        }
      },
      token);
}

bool
CSGRegion::nextRegionOpIsIdentical(const std::string & region_op_string,
                                   unsigned int postfix_token_index) const
{
  for (unsigned int i = postfix_token_index; i < _postfix_tokens.size(); ++i)
  {
    const auto postfix_token_string = postfixTokenToString(_postfix_tokens[i]);
    if (postfix_token_string.find("regionOperator::") != std::string::npos)
      return (postfix_token_string == region_op_string);
  }
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
  const bool region_str_eq = this->toInfixString() == other.toInfixString();
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
