//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "hit/hit.h"

#include <string>
#include <vector>

class InputParameters;

namespace Moose::ParameterExtraction
{
/**
 * Helper struct that contains the context filled during the extract methods
 */
struct ExtractionInfo
{
  /// The errors collected during extraction
  std::vector<hit::ErrorMessage> errors;
  /// The variables that were extracted
  std::vector<std::string> extracted_variables;
  /// The deprecation warnings (object type/param name) -> (message)
  std::map<std::string, std::string> deprecated_params;
};

/**
 * Attempt to extract values from input starting with the section in input in \p section_node
 * based on the contents of the passed InputParameters \p p from the root hit node \p root.
 *
 * If \p section_node is not provided, only the global parameters will be checked
 */
ExtractionInfo
extract(const hit::Node & root, const hit::Node * const section_node, InputParameters & p);

/**
 * Attempt to extract values from input starting with the section in input defined
 * by the fullpath \p prefix based on the contents of the passed InputParameters \p p
 * from the root hit node \p root.
 */
ExtractionInfo extract(const hit::Node & root, const std::string & prefix, InputParameters & p);
}
