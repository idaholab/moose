//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "nlohmann/json.h"
#include <map>
#include <string>
#include <vector>
#include <sstream>

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
  std::string toString(const nlohmann::json & root);

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
                const nlohmann::json & block,
                bool is_typeblock = false,
                const std::string & parent_name = "",
                const nlohmann::json & parameters_in = nlohmann::json(nullptr),
                const nlohmann::json & subblocks_in = nlohmann::json(nullptr));

  /**
   * adds all parameters from a given block
   * @param params - json holding data for all of the given block's parameters
   */
  void addParameters(const nlohmann::json & params);

  const int _spaces;
  int _level;
  std::ostringstream _stream;
  std::map<std::string, std::vector<std::string>> _assoc_types_map;
  nlohmann::json _global_params;
};
