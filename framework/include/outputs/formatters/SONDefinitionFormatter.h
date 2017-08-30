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

#ifndef SONDEFINITIONFORMATTER_H
#define SONDEFINITIONFORMATTER_H
#include "json/json.h"
#include <map>
#include <string>
#include <vector>

typedef moosecontrib::Json::Value JsonVal;

/**
 * This class produces a dump of the InputFileParameters in the Standard Object
 * Notation (SON) format for use by the Hierarchical Input Validation Engine
 * (HIVE) in the NEAMS Workbench. It takes its input from JsonSyntaxTree.
 */
class SONDefinitionFormatter
{

public:
  SONDefinitionFormatter();

  /**
   * returns a string representation of the tree in input file format
   * @param root - the root node of the tree to output
   */
  std::string toString(const JsonVal & root);

protected:
  /**
   * adds a line to the output with the proper indentation automatically
   * @param line - the line to add
   */
  void addLine(const std::string & line);

  /**
   * adds a new block to the output
   * @param block_name    - name of the block
   * @param block         - json holding data for the block
   * @param parameters_in - if a typeblock, the parameters for inheritance
   * @param subblocks_in  - if a typeblock, the subblocks for inheritance
   * @param is_typeblock  - true only if block being added is a typeblock
   */
  void addBlock(const std::string & block_name,
                const JsonVal & block,
                bool is_typeblock = false,
                const std::string & parent_name = "",
                const JsonVal & parameters_in = JsonVal::null,
                const JsonVal & subblocks_in = JsonVal::null);

  /**
   * adds all parameters from a given block
   * @param params - json holding data for all of the given block's parameters
   */
  void addParameters(const JsonVal & params);

  /**
   * return backtrack relative path from the current level to the document root
   */
  std::string backtrack()
  {
    std::string backtrack_path;
    for (int i = 0; i < _level; ++i)
      backtrack_path += "../";
    return backtrack_path;
  }

  JsonVal _globalparams;
  const int _spaces;
  int _level;
  std::ostringstream _stream;
  std::map<std::string, std::vector<std::string>> _assoc_types_map;
  std::map<std::string, std::string> _json_path_regex_replacement_map = {
      {"/star/subblock_types/([A-Za-z0-9_]*)/", "/\\1_type/"},
      {"[A-Za-z0-9_]*/types/([A-Za-z0-9_]*)/", "\\1_type/"},
      {"/actions/[A-Za-z0-9_]*/parameters/", "/"},
      {"/parameters/", "/"},
      {"/subblocks/", "/"}};
};

#endif /* SONDEFINITIONFORMATTER_H */
