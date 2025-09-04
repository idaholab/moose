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

void
appendErrorMessages(std::vector<hit::ErrorMessage> & to, const hit::Error & error)
{
  appendErrorMessages(to, error.error_messages);
}

std::string
joinErrorMessages(const std::vector<hit::ErrorMessage> & error_messages)
{
  std::vector<std::string> values;
  for (const auto & em : error_messages)
    values.push_back(em.prefixed_message);
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
parseError(const hit::Node & root,
           std::vector<hit::ErrorMessage> messages,
           const bool throw_on_error)
{
  // Few things about command line arguments...
  // 1. We don't care to add line and column context for CLI args, because
  //    it doesn't make sense. We go from the full CLI args and pull out
  //    the HIT parameters so "line" 1 might not even be command line
  //    argument 1. So, remove line/column context from all CLI args.
  // 2. Whenever we have a parameter in input that then gets overridden
  //    by a command line argument, under the hood we're merging two
  //    different HIT trees. However, WASP doesn't currently update the
  //    "filename" context for the updated parameter. Which means that
  //    a param that is in input and then overridden by CLI will have
  //    its location as in input. Which isn't true. So we get around this
  //    by searching the independent CLI args tree for params that we have
  //    errors for. If the associated path is also in CLI args, we manually
  //    set its error to come from CLI args. This should be fixed in
  //    the future with a WASP update.
  if (const auto global_params_node = queryGlobalParamsNode(root))
    for (auto & em : messages)
      if (em.node && isGlobal(*global_params_node, *em.node))
        em = hit::ErrorMessage(em.message, "CLI_ARGS");

  if (throw_on_error)
    throw ParseError(messages);
  else
    ::mooseError(joinErrorMessages(messages));
}

}
