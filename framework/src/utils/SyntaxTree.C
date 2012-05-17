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
SyntaxTree::print(const std::string &search_string) const
{
  bool found=false;
  std::string output;

  if (_root)
    output = _root->print(-1, search_string, found);

  if (found)
    return preamble() + output + postscript();
  else
    return "";
}

SyntaxTree::TreeNode::TreeNode(const std::string &name, const SyntaxTree &syntax_tree, const std::string *action, InputParameters *params, TreeNode *parent) :
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
SyntaxTree::TreeNode::print(short depth, const std::string &search_string, bool &found) const
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

  std::string indent((depth+1)*2, ' ');

  std::multimap<std::string, InputParameters *>::const_iterator it = _moose_object_params.begin();
  do
  {
    bool local_found = false;
    std::string local_out;

    // Compare the block name, if it's matched we are going to pass an empty search string
    // which means match ALL parameters
    std::string local_search_string;
    if (name == search_string)
      found = true;
    else
      local_search_string = search_string;

    local_out += _syntax_tree.printBlockOpen(name, depth, type);

    for (std::multimap<std::string, InputParameters *>::const_iterator a_it = _action_params.begin(); a_it != _action_params.end(); ++a_it)
      if(a_it->first != "EmptyAction")
      {
        local_out += _syntax_tree.printParams(*a_it->second, depth, local_search_string, local_found);
        found |= local_found;   // Update the current frame's found variable
        //DEBUG
        // std::cout << "\n" << indent << "(" << ait->first << ")";
        //DEBUG
      }

    if (it != _moose_object_params.end())
    {
      local_out += _syntax_tree.printParams(*it->second, depth, local_search_string, local_found);
      found |= local_found;
      //DEBUG
      // std::cout << "\n" << indent << "{" << it->first << "}";
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
