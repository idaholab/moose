//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SyntaxTree.h"

#include "InputParameters.h"
#include "MooseUtils.h"
#include "Parser.h"

#include <algorithm>
#include <cctype>

SyntaxTree::SyntaxTree(bool use_long_names)
  : SyntaxFormatterInterface(), _use_long_names(use_long_names)
{
}

SyntaxTree::~SyntaxTree() = default;

void
SyntaxTree::insertNode(std::string syntax,
                       const std::string & action,
                       bool is_action_params,
                       InputParameters * params)
{
  if (!_root)
    _root = std::make_unique<TreeNode>("", *this);

  _root->insertNode(syntax, action, is_action_params, params);
}

std::string
SyntaxTree::print(const std::string & search_string)
{
  bool found = false;
  std::string output;

  // Clear the list of "seen" parameters before printing the tree
  _params_printed.clear();

  if (_root)
    output = _root->print(-1, search_string, found);

  if (found)
    return preamble() + output + postscript();
  else
    return "";
}

void
SyntaxTree::seenIt(const std::string & prefix, const std::string & item)
{
  _params_printed.insert(prefix + item);
}

bool
SyntaxTree::haveSeenIt(const std::string & prefix, const std::string & item) const
{
  return _params_printed.find(prefix + item) != _params_printed.end();
}

SyntaxTree::TreeNode::TreeNode(const std::string & name,
                               SyntaxTree & syntax_tree,
                               const std::string * action,
                               InputParameters * params,
                               TreeNode * parent)
  : _name(name), _parent(parent), _syntax_tree(syntax_tree)
{
  if (action)
    _action_params.emplace(*action, std::make_unique<InputParameters>(*params));
}

SyntaxTree::TreeNode::~TreeNode() = default;

void
SyntaxTree::TreeNode::insertNode(std::string & syntax,
                                 const std::string & action,
                                 bool is_action_params,
                                 InputParameters * params)
{
  std::string::size_type pos = syntax.find_first_of("/");
  std::string item;
  bool is_leaf = true;

  item = syntax.substr(0, pos);
  if (pos != std::string::npos)
  {
    syntax = syntax.substr(pos + 1);
    is_leaf = false;
  }

  bool node_created = false;
  if (_children.find(item) == _children.end())
  {
    _children[item] = std::make_unique<TreeNode>(
        item, _syntax_tree, is_leaf && is_action_params ? &action : NULL, params, this);
    if (is_leaf && !is_action_params)
      _children[item]->insertParams(action, is_action_params, params);
    node_created = true;
  }

  if (!is_leaf)
    _children[item]->insertNode(syntax, action, is_action_params, params);
  else if (!node_created)
    _children[item]->insertParams(action, is_action_params, params);
}

void
SyntaxTree::TreeNode::insertParams(const std::string & action,
                                   bool is_action_params,
                                   InputParameters * params)
{
  if (is_action_params)
    _action_params.emplace(action, std::make_unique<InputParameters>(*params));
  else
    _moose_object_params.emplace(action, std::make_unique<InputParameters>(*params));
}

std::string
SyntaxTree::TreeNode::print(short depth, const std::string & search_string, bool & found)
{
  std::string doc = "";
  std::string long_name(getLongName());
  std::string name(_syntax_tree.isLongNames() ? long_name : _name);
  std::string out;

  if (depth < 0)
  {
    for (const auto & c_it : _children)
    {
      bool local_found = false;
      std::string local_out(c_it.second->print(depth + 1, search_string, local_found));
      found |= local_found; // Update the current frame's found variable
      if (local_found)
        out += local_out;
    }
    return out;
  }

  // GlobalParamsAction is special - we need to just always print it out
  // if (_name == "GlobalParamsAction")
  //   found = true;

  std::string indent((depth + 1) * 2, ' ');

  std::multimap<std::string, std::unique_ptr<InputParameters>>::const_iterator it =
      _moose_object_params.begin();
  do
  {
    bool local_found = false;
    std::string local_out;

    // Compare the block name, if it's matched we are going to pass an empty search string
    // which means match ALL parameters
    std::string local_search_string;
    if (MooseUtils::wildCardMatch(name, search_string))
      found = true;
    else
      local_search_string = search_string;

    if (it != _moose_object_params.end())
      doc = it->second->getClassDescription();
    local_out += _syntax_tree.printBlockOpen(name, depth, doc);

    for (const auto & a_it : _action_params)
      if (a_it.first != "EmptyAction")
      {
        local_out += _syntax_tree.printParams(
            name, long_name, *(a_it.second), depth, local_search_string, local_found);
        found |= local_found; // Update the current frame's found variable
        // DEBUG
        // Moose::out << "\n" << indent << "(" << ait->first << ")";
        // DEBUG
      }

    if (it != _moose_object_params.end())
    {
      local_out += _syntax_tree.printParams(
          name, long_name, *it->second, depth, local_search_string, local_found);
      found |= local_found;
      // DEBUG
      // Moose::out << "\n" << indent << "{" << it->first << "}";
      // DEBUG
    }

    local_out += _syntax_tree.preTraverse(depth);

    for (const auto & c_it : _children)
    {
      bool child_found = false;
      std::string child_out(c_it.second->print(depth + 1, local_search_string, child_found));
      found |= child_found; // Update the current frame's found variable

      if (child_found)
        local_out += child_out;
    }

    local_out += _syntax_tree.printBlockClose(name, depth);

    if (found)
      out += local_out;

    // If there are no moose object params then we have to be careful about how we
    // increment the iterator.  We only want to increment if we aren't already
    // at the end.
  } while (it != _moose_object_params.end() && ++it != _moose_object_params.end());

  return out;
}

std::string
SyntaxTree::TreeNode::getLongName(const std::string & delim) const
{
  if (_parent)
    return _parent->getLongName(delim) + delim + _name;
  else
    return _name;
}

bool
SyntaxTree::isLongNames() const
{
  return _use_long_names;
}
