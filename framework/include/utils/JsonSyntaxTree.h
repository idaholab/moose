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
#include "FileLineInfo.h"
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
   * @param is_action Wheter it is an action
   * @param params The InputParameters to add to the tree
   * @param lineinfo The FileLineInfo where the action/path was registered
   * @return Whether the parameters were added to the tree (ie if it matched the search string).
   */
  bool addParameters(const std::string & parent_path,
                     const std::string & path,
                     bool is_type,
                     const std::string & action,
                     bool is_action,
                     InputParameters * params,
                     const FileLineInfo & lineinfo);

  /**
   * Add a task to the tree
   * @param path The path of the action
   * @param action Name of the action
   * @param task Name of the task
   * @param lineinfo The FileLineInfo where the action/task was registered
   */
  void addActionTask(const std::string & path,
                     const std::string & action,
                     const std::string & task,
                     const FileLineInfo & lineinfo);
  /**
   * Get the root of the tree.
   * @return The top level Json::Value holding the tree.
   */
  const moosecontrib::Json::Value & getRoot() const { return _root; }

  /**
   * Add an associated type to a block
   * @param path Path of the block
   * @param type Type name to associate the block with
   */
  void addSyntaxType(const std::string & path, const std::string type);

protected:
  std::string buildOptions(const std::iterator_traits<InputParameters::iterator>::value_type & p);

  std::string prettyCppType(const std::string & cpp_type);
  std::string basicCppType(const std::string & cpp_type);

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
