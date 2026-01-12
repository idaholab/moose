//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "nlohmann/json.h"
#include <variant>
#include <vector>

namespace JsonOutputUtils
{

/**
 * @brief Helper function to convert std::any to JSON type. Supported data types that can be
 * converted are: int, unsigned int, Real, std::string, and bool, and vectors of these kinds.
 *
 * @param data std::any object to convert
 * @return nlohmann::json representation of the data
 */
nlohmann::json anyToJson(const std::any & data);

/**
 * Type definition for a variant that can hold all the supported types for lattice attributes
 */
typedef std::variant<int,
                     unsigned int,
                     std::string,
                     Real,
                     bool,
                     std::vector<int>,
                     std::vector<unsigned int>,
                     std::vector<std::string>,
                     std::vector<Real>,
                     std::vector<bool>>
    AttributeVariant;

/**
 * @brief Helper function to convert AttributeVariant to JSON type.
 *
 * @param data AttributeVariant object to convert
 * @return nlohmann::json representation of the data
 */
nlohmann::json variantToJson(const AttributeVariant & data);
}
