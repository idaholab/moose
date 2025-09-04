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
///@{
void appendErrorMessages(std::vector<hit::ErrorMessage> & to,
                         const std::vector<hit::ErrorMessage> & from);
void appendErrorMessages(std::vector<hit::ErrorMessage> & to, const hit::Error & error);
///@}

/**
 * Helper for combining error messages into a single, newline separated message
 */
std::string joinErrorMessages(const std::vector<hit::ErrorMessage> & error_messages);

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
 * @param root The root node that was parsed from
 * @param messages The error messages
 * @param throw_on_error Whether or not to throw a ParseError instead of calling mooseError
 */
void parseError(const hit::Node & root,
                std::vector<hit::ErrorMessage> messages,
                const bool throw_on_error);
}
