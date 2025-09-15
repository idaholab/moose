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

#include <vector>

namespace Moose::ParseUtils
{
/**
 * Helper for accumulating errors from a walker into an accumulation of errors
 */
void appendErrorMessages(std::vector<hit::ErrorMessage> & to,
                         const std::vector<hit::ErrorMessage> & from);

/**
 * Helper for combining error messages into a single, newline separated message
 *
 * The \p augment_node_errors argument disables extra augmentation of node
 * errors in the event of CLI arguments; this must be disabled whenever
 * reporting errors for pure syntax issues as the tree might not be in
 * a fully valid state
 *
 * @param error_messages The error messages
 * @param augment_node_errors Whether or not to try to augment node errors for CLI arguments
 */
std::string joinErrorMessages(const std::vector<hit::ErrorMessage> & error_messages,
                              const bool augment_node_errors = true);

/**
 * Obtain the GlobalParameters section from the given hit root node if available.
 */
const hit::Node * queryGlobalParamsNode(const hit::Node & root);

/**
 * Check whether or not \p node is a parameter within the global parameter section
 * defined by \p global_params_node
 */
bool isGlobal(const hit::Node & global_params_node, const hit::Node & node);

/**
 * Check whether or not \p node is a parameter within the global parameter section
 */
bool isGlobal(const hit::Node & node);

/**
 * Derived exception for a collection of parse error messages
 */
struct ParseError : public hit::Error
{
  ParseError() = delete;
  ParseError(const std::vector<hit::ErrorMessage> & error_messages);
};

/**
 * Report a parse error.
 *
 * Used by Parser, Builder, and MooseApp::extractApplicationParams.
 *
 * The \p augment_node_errors argument disables extra augmentation of node
 * errors in the event of CLI arguments; this must be disabled whenever
 * reporting errors for pure syntax issues as the tree might not be in
 * a fully valid state
 *
 * @param messages The error messages
 * @param throw_on_error Whether or not to throw a ParseError instead of calling mooseError
 * @param augment_node_errors Whether or not to try to augment node errors for CLI arguments
 */
void parseError(const std::vector<hit::ErrorMessage> & messages,
                const bool throw_on_error,
                const bool augment_node_errors = true);

/**
 * Helper for finding an equivalent command line hit node.
 *
 * This is needed until idaholab/moose#31461 is resolved due to
 * issues with merging command line hit parameters into the
 * main input hit tree, as the filename paths are not properly
 * updated upon merge.
 */
const hit::Node * queryCommandLineNode(const hit::Node & node, const hit::Node & command_line_root);
}
