//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

// Forward declarations
class InputParameters;
class SyntaxTree;

class SyntaxTree : public SyntaxFormatterInterface
{
public:
  SyntaxTree(bool use_long_names = false);
  virtual ~SyntaxTree();

  void insertNode(std::string syntax,
                  const std::string & action,
                  bool is_action_params = true,
                  InputParameters * params = NULL);

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
    TreeNode(const std::string & name,
             SyntaxTree & syntax_tree,
             const std::string * action = NULL,
             InputParameters * params = NULL,
             TreeNode * parent = NULL);
    ~TreeNode();

    void insertNode(std::string & syntax,
                    const std::string & action,
                    bool is_action_params = true,
                    InputParameters * params = NULL);
    std::string print(short depth, const std::string & search_string, bool & found);

    std::string getLongName(const std::string & delim = "/") const;

  protected:
    void insertParams(const std::string & action,
                      bool is_action_params,
                      InputParameters * params = NULL);

    std::map<std::string, std::unique_ptr<TreeNode>> _children;
    std::multimap<std::string, std::unique_ptr<InputParameters>> _action_params;
    std::multimap<std::string, std::unique_ptr<InputParameters>> _moose_object_params;
    std::string _name;
    TreeNode * _parent;
    SyntaxTree & _syntax_tree;
  };

  bool isLongNames() const;

  std::unique_ptr<TreeNode> _root;
  bool _use_long_names;

private:
  std::set<std::string> _params_printed;
};
