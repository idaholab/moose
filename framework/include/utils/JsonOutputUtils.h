//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "nlohmann/json.h"

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
}
