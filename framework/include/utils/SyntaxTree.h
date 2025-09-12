//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SyntaxFormatterInterface.h"

#include <map>
#include <memory>
#include <set>
#include <string>

class InputParameters;
class SyntaxTree;

class SyntaxTree : public SyntaxFormatterInterface
{
public:
  SyntaxTree(const bool use_long_names = false);
  virtual ~SyntaxTree() = default;

  void insertNode(const std::string & syntax,
                  const std::string & action,
                  const bool is_action_params,
                  const InputParameters & params);

  std::string print(const std::string & search_string);

  void seenIt(const std::string & prefix, const std::string & item);
  bool haveSeenIt(const std::string & prefix, const std::string & item) const;

protected:
  /**
   * This class represents a single node in our tree
   */
  class TreeNode
  {
  public:
    TreeNode(SyntaxTree & syntax_tree);
    TreeNode(const std::string & name,
             SyntaxTree & syntax_tree,
             const std::string * const action,
             const InputParameters & params,
             const TreeNode & parent);
    ~TreeNode() = default;

    void insertNode(std::string syntax,
                    const std::string & action,
                    const bool is_action_params,
                    const InputParameters & params);
    std::string print(const short depth, const std::string & search_string, bool & found);

    std::string getLongName() const;

  protected:
    void insertParams(const std::string & action,
                      const bool is_action_params,
                      const InputParameters & params);

    std::map<std::string, std::unique_ptr<TreeNode>> _children;
    std::multimap<std::string, std::unique_ptr<InputParameters>> _action_params;
    std::multimap<std::string, std::unique_ptr<InputParameters>> _moose_object_params;
    const std::string _name;
    const TreeNode * const _parent;
    SyntaxTree & _syntax_tree;
  };

  bool isLongNames() const;

  std::unique_ptr<TreeNode> _root;
  const bool _use_long_names;

private:
  std::set<std::string> _params_printed;
};
