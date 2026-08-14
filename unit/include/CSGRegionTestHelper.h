//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseUnitUtils.h"

// helper function to convert infix JSON object to string representation
inline std::string
infixJSONToString(nlohmann::json infix_json)
{
  auto json_string = infix_json.dump();
  // Remove quotation marks from string
  json_string.erase(std::remove(json_string.begin(), json_string.end(), '\"'), json_string.end());
  // Replace commas with a space
  std::replace(json_string.begin(), json_string.end(), ',', ' ');
  // Replace square brackets with parentheses
  std::replace(json_string.begin(), json_string.end(), '[', '(');
  std::replace(json_string.begin(), json_string.end(), ']', ')');
  return json_string;
}
