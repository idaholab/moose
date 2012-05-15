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

#ifndef SYNTAXTREE_H
#define SYNTAXTREE_H

#include "SyntaxFormatterInterface.h"

#include <string>
#include <vector>
#include <map>
#include <cstddef>

// Forward declarations
class InputParameters;
class SyntaxTree;

class SyntaxTree: public SyntaxFormatterInterface
{
public:
  SyntaxTree(bool use_long_names=false);

  virtual ~SyntaxTree();

  void insertNode(std::string syntax, const std::string &action, bool is_action_params=true,
                  InputParameters *params=NULL);

  virtual void print() const;

protected:
  /**
   * This class represents a single node in our tree
   */
  class TreeNode
  {
  public:
    TreeNode(const std::string &name, const SyntaxTree &syntax_tree, const std::string *action=NULL, InputParameters *params=NULL, TreeNode *parent=NULL);
    virtual ~TreeNode();

    void insertNode(std::string &syntax, const std::string &action, bool is_action_params=true,
                    InputParameters *params=NULL);
    void print(short depth) const;

    std::string getLongName(const std::string &delim="/") const;

  protected:
    void insertParams(const std::string &action, bool is_action_params, InputParameters *params=NULL);

    std::map<std::string, TreeNode *> _children;
    std::map<std::string, InputParameters *> _action_params;
    std::map<std::string, InputParameters *> _moose_object_params;
    std::string _name;
    TreeNode *_parent;
    const SyntaxTree &_syntax_tree;
  };

  bool isLongNames() const { return _use_long_names; }

  TreeNode *_root;
  bool _use_long_names;
};


#endif //SYNTAXTREE_H
