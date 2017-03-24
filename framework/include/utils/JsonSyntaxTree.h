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

#ifndef JSONSYNTAXTREE_H
#define JSONSYNTAXTREE_H

#include "InputParameters.h"
#include "json/json.h"
#include <string>
#include <vector>

/**
 * Holds the syntax in a Json::Value tree
 */
class JsonSyntaxTree
{
public:
  JsonSyntaxTree(const std::string & search_string);
  virtual ~JsonSyntaxTree() {}

  /**
   * Add parameters to the tree
   * @param parent_path The parent syntax path that the action belongs to
   * @param path The path of the action
   * @param is_type Whether this belongs to a "<type>" or not
   * @param action Name of the action
   * @param task_name Task name associated with these parameters
   * @param is_action Wheter it is an action
   * @param params The InputParameters to add to the tree
   */
  void addParameters(const std::string & parent_path,
                     const std::string & path,
                     bool is_type,
                     const std::string & action,
                     const std::string & task_name,
                     bool is_action,
                     InputParameters * params);

  const moosecontrib::Json::Value & getRoot() const { return _root; }

  /**
   * Add an associated type to a block
   * @param path Path of the block
   * @param type Type name to associate the block with
   */
  void addSyntaxType(const std::string & path, const std::string type);

protected:
  std::string buildOptions(const std::iterator_traits<InputParameters::iterator>::value_type & p);

  std::string
  buildOutputString(const std::iterator_traits<InputParameters::iterator>::value_type & p);
  static std::vector<std::string> splitPath(const std::string & path);
  moosecontrib::Json::Value &
  getJson(const std::string & parent, const std::string & path, bool is_type);
  moosecontrib::Json::Value & getJson(const std::string & path);
  moosecontrib::Json::Value _root;
  std::string _search;
};

#endif // JSONSYNTAXTREE_H
