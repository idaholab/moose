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

SyntaxTree::SyntaxTree() :
    SyntaxFormatterInterface(),
    _root(NULL)
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
    _root = new TreeNode("/", *this);

  _root->insertNode(syntax, action, is_action_params, params);
}

void
SyntaxTree::print() const
{
  if (_root)
    _root->print(-1);
}

SyntaxTree::TreeNode::TreeNode(const std::string &name, const SyntaxTree &syntax_tree, const std::string *action, InputParameters *params) :
    _name(name),
    _syntax_tree(syntax_tree)
{
  if (action)
    _action_params[*action] = new InputParameters(*params);
}

SyntaxTree::TreeNode::~TreeNode()
{
  for (std::map<std::string, InputParameters *>::iterator it = _action_params.begin();
       it != _action_params.end(); ++it)
    delete it->second;

  for (std::map<std::string, InputParameters *>::iterator it = _moose_object_params.begin();
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
    _children[item] = new TreeNode(item, _syntax_tree, is_leaf ? &action : NULL, params);
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
    _action_params[action] = new InputParameters(*params);
  else
    _moose_object_params[action] = new InputParameters(*params);
}

void
SyntaxTree::TreeNode::print(short depth) const
{
  std::string type;

  if (depth < 0)
  {
    for (std::map<std::string, TreeNode *>::const_iterator it = _children.begin(); it != _children.end(); ++it)
      it->second->print(depth+1);
    return;
  }

  if (_moose_object_params.begin() == _moose_object_params.end())
  {
    _syntax_tree.printBlockOpen(_name, depth, type);

    for (std::map<std::string, InputParameters *>::const_iterator ait = _action_params.begin(); ait != _action_params.end(); ++ait)
      _syntax_tree.printParams(*ait->second, depth);

    for (std::map<std::string, TreeNode *>::const_iterator it = _children.begin(); it != _children.end(); ++it)
      it->second->print(depth+1);

    _syntax_tree.printBlockClose(_name, depth);
  }
  else
  {
    for (std::map<std::string, InputParameters *>::const_iterator it = _moose_object_params.begin(); it != _moose_object_params.end(); ++it)
    {
      _syntax_tree.printBlockOpen(_name, depth, type);

      for (std::map<std::string, InputParameters *>::const_iterator ait = _action_params.begin(); ait != _action_params.end(); ++ait)
        _syntax_tree.printParams(*ait->second, depth);

      _syntax_tree.printParams(*it->second, depth);

      for (std::map<std::string, TreeNode *>::const_iterator it = _children.begin(); it != _children.end(); ++it)
        it->second->print(depth+1);

      _syntax_tree.printBlockClose(_name, depth);
    }
  }
}
