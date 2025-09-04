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
 * Attempt to extract values from input into parameters starting with the given section node.
 *
 * If \p section_node is not provided, only global parameters will be checked.
 *
 * \p command_line_root is an optional argument that allows for the correct
 * association of parameter -> hit node and error -> hit node when values
 * are extracted from the command line. Once #31461 is resolved, this shoul
 * no longer be needed. Currently, the command line tree is merged into
 * the input tree, but the filenames are not properly updated during this merge.
 * Thus, parameters that are overridden via command line will incorrectly
 * be associated with the input file. See idaholab/moose#31461.
 *
 * @param root The root hit node to extract from
 * @param command_line_root The command line root node, if any (see above for info)
 * @param section_node The section to parse from, if any
 * @param params The parameters to fill into
 * @return The information about what was extracted, including errors
 */
ExtractionInfo extract(const hit::Node & root,
                       const hit::Node * const command_line_root,
                       const hit::Node * const section_node,
                       InputParameters & params);

/**
 * Attempt to extract values from input into parameters starting with the given prefix.
 *
 * See the extraction method above for information on \p command_line_root.
 *
 * @param root The root hit node to extract from
 * @param command_line_root The command line root node, if any (see above for info)
 * @param prefix The prefix of the section to parse from
 * @param params The parameters to fill into
 * @return The information about what was extracted, including errors
 */
ExtractionInfo extract(const hit::Node & root,
                       const hit::Node * const command_line_root,
                       const std::string & prefix,
                       InputParameters & params);

ExtractionInfo
extract(const hit::Node & root, const std::string & prefix, InputParameters & params);
}
