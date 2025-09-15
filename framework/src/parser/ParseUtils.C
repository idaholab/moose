//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParseUtils.h"

#include "MooseUtils.h"
#include "Moose.h"
#include "MooseError.h"

namespace Moose::ParseUtils
{
void
appendErrorMessages(std::vector<hit::ErrorMessage> & to,
                    const std::vector<hit::ErrorMessage> & from)
{
  to.insert(to.end(), from.begin(), from.end());
}

std::string
joinErrorMessages(const std::vector<hit::ErrorMessage> & error_messages,
                  const bool augment_node_errors /* = true */)
{
  std::vector<std::string> values;
  for (const auto & em : error_messages)
  {
    // Attempt to provide a better prefix if requested
    if (augment_node_errors && em.node)
      if (const auto hit_prefix = Moose::hitMessagePrefix(*em.node))
      {
        values.push_back(*hit_prefix + " " + em.message);
        continue;
      }

    values.push_back(em.prefixed_message);
  }
  return MooseUtils::stringJoin(values, "\n");
}

const hit::Node *
queryGlobalParamsNode(const hit::Node & root)
{
  mooseAssert(root.isRoot(), "Is not the root node");
  const auto node = root.find(Moose::global_params_syntax);
  if (node && node->parent() == &root && node->type() == hit::NodeType::Section)
    return node;
  return nullptr;
}

bool
isGlobal(const hit::Node & global_params_node, const hit::Node & node)
{
  return node.type() == hit::NodeType::Field && node.parent() == &global_params_node;
}

bool
isGlobal(const hit::Node & node)
{
  const auto global_params_node = queryGlobalParamsNode(*node.root());
  return global_params_node ? isGlobal(*global_params_node, node) : false;
}

ParseError::ParseError(const std::vector<hit::ErrorMessage> & error_messages)
  : hit::Error(error_messages)
{
}

void
parseError(const std::vector<hit::ErrorMessage> & messages,
           const bool throw_on_error,
           const bool augment_node_errors)
{
  if (throw_on_error)
    throw ParseError(messages);
  else
    ::mooseError(joinErrorMessages(messages, augment_node_errors));
}

const hit::Node *
queryCommandLineNode(const hit::Node & node, const hit::Node & command_line_root)
{
  const auto command_line_node = command_line_root.find(node.fullpath());
  if (command_line_node && node.type() == command_line_node->type() &&
      (node.type() != hit::NodeType::Field ||
       node.param<std::string>() == command_line_node->param<std::string>()))
    return command_line_node;
  return nullptr;
}
}
