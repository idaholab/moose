/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SyntaxTree.h"

#include "InputParameters.h"
#include "Parser.h"

#include <algorithm>
#include <cctype>

SyntaxTree::SyntaxTree(bool use_long_names) :
    SyntaxFormatterInterface(),
    _root(NULL),
    _use_long_names(use_long_names)
{
}

SyntaxTree::~SyntaxTree()
{
  delete (_root);
}

void
SyntaxTree::insertNode(std::string syntax, const std::string &action, bool is_action_params, InputParameters *params)
{
  if (_root == NULL)
    _root = new TreeNode("", *this);

  _root->insertNode(syntax, action, is_action_params, params);
}

std::string
SyntaxTree::print(const std::string &search_string)
{
  bool found=false;
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
SyntaxTree::seenIt(const std::string &prefix, const std::string &item)
{
  _params_printed.insert(prefix + item);
}

bool
SyntaxTree::haveSeenIt(const std::string &prefix, const std::string &item) const
{
  return _params_printed.find(prefix + item) != _params_printed.end();
}

SyntaxTree::TreeNode::TreeNode(const std::string &name, SyntaxTree &syntax_tree, const std::string *action, InputParameters *params, TreeNode *parent) :
    _name(name),
    _parent(parent),
    _syntax_tree(syntax_tree)
{
  if (action)
    _action_params.insert(std::make_pair(*action, new InputParameters(*params)));
}

SyntaxTree::TreeNode::~TreeNode()
{
  for (std::multimap<std::string, InputParameters *>::iterator it = _action_params.begin();
       it != _action_params.end(); ++it)
    delete it->second;

  for (std::multimap<std::string, InputParameters *>::iterator it = _moose_object_params.begin();
       it != _moose_object_params.end(); ++it)
    delete it->second;

  for (std::map<std::string, TreeNode *>::iterator it = _children.begin(); it != _children.end(); ++it)
    delete it->second;
}

void
SyntaxTree::TreeNode::insertNode(std::string &syntax, const std::string &action, bool is_action_params,
                                 InputParameters *params)
{
  std::string::size_type pos = syntax.find_first_of("/");
  std::string item;
  bool is_leaf = true;

  item = syntax.substr(0, pos);
  if (pos != std::string::npos)
  {
    syntax = syntax.substr(pos+1);
    is_leaf = false;
  }

  bool node_created = false;
  if (_children.find(item) == _children.end())
  {
    _children[item] = new TreeNode(item, _syntax_tree, is_leaf && is_action_params ? &action : NULL, params, this);
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
SyntaxTree::TreeNode::insertParams(const std::string &action, bool is_action_params, InputParameters *params)
{
  if (is_action_params)
    _action_params.insert(std::make_pair(action, new InputParameters(*params)));
  else
    _moose_object_params.insert(std::make_pair(action, new InputParameters(*params)));
}

std::string
SyntaxTree::TreeNode::print(short depth, const std::string &search_string, bool &found)
{
  std::string type;
  std::string long_name(getLongName());
  std::string name(_syntax_tree.isLongNames() ? long_name : _name);
  std::string out;

  if (depth < 0)
  {
    for (std::map<std::string, TreeNode *>::const_iterator c_it = _children.begin(); c_it != _children.end(); ++c_it)
    {
      bool local_found = false;
      std::string local_out (c_it->second->print(depth+1, search_string, local_found));
      found |= local_found;  // Update the current frame's found variable
      if (local_found)
        out += local_out;
    }
    return out;
  }

  // GlobalParamsAction is special - we need to just always print it out
//  if (_name == "GlobalParamsAction")
//    found = true;

  std::string indent((depth+1)*2, ' ');

  std::multimap<std::string, InputParameters *>::const_iterator it = _moose_object_params.begin();
  do
  {
    bool local_found = false;
    std::string local_out;

    // Compare the block name, if it's matched we are going to pass an empty search string
    // which means match ALL parameters
    std::string local_search_string;
    if (wildCardMatch(name, search_string))
      found = true;
    else
      local_search_string = search_string;

    local_out += _syntax_tree.printBlockOpen(name, depth, type);

    for (std::multimap<std::string, InputParameters *>::const_iterator a_it = _action_params.begin(); a_it != _action_params.end(); ++a_it)
      if (a_it->first != "EmptyAction")
      {
        local_out += _syntax_tree.printParams(name, *a_it->second, depth, local_search_string, local_found);
        found |= local_found;   // Update the current frame's found variable
        //DEBUG
        // Moose::out << "\n" << indent << "(" << ait->first << ")";
        //DEBUG
      }

    if (it != _moose_object_params.end())
    {
      local_out += _syntax_tree.printParams(name, *it->second, depth, local_search_string, local_found);
      found |= local_found;
      //DEBUG
      // Moose::out << "\n" << indent << "{" << it->first << "}";
      //DEBUG
    }

    local_out += _syntax_tree.preTraverse(depth);

    for (std::map<std::string, TreeNode *>::const_iterator c_it = _children.begin(); c_it != _children.end(); ++c_it)
    {
      bool child_found = false;
      std::string child_out (c_it->second->print(depth+1, local_search_string, child_found));
      found |= child_found;   // Update the current frame's found variable

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
SyntaxTree::TreeNode::getLongName(const std::string &delim) const
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

bool
SyntaxTree::wildCardMatch(std::string name, std::string search_string)
{
  // Assume that an empty string matches anything
  if (search_string == "")
    return true;

  // transform to lower for case insenstive matching
  std::transform(name.begin(), name.end(), name.begin(), (int(*)(int))std::toupper);
  std::transform(search_string.begin(), search_string.end(), search_string.begin(), (int(*)(int))std::toupper);

  // exact match!
  if (search_string.find("*") == std::string::npos)
    return search_string == name;

  // wildcard
  std::vector<std::string> tokens;
  MooseUtils::tokenize(search_string, tokens, 1, "*");

  size_t pos = 0;
  for (unsigned int i=0; i<tokens.size() && pos != std::string::npos; ++i)
  {
    pos = name.find(tokens[i], pos);
    // See if we have a leading wildcard
    if (search_string[0] != '*' && i == 0 && pos != 0)
      return false;
  }

  if (pos != std::string::npos)
  {
    // Now see if we have a trailing wildcard
    size_t last_token_length = tokens[tokens.size()-1].length();
    if (search_string[search_string.length()-1] == '*' || pos == name.size() - last_token_length)
      return true;
    else
      return false;
  }
  else
    return false;
}
