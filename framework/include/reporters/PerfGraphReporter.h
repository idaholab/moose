//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

/**
 * Reports the full graph from the PerfGraph
 */
class PerfGraphReporter : public GeneralReporter
{
public:
  static InputParameters validParams();

  PerfGraphReporter(const InputParameters & parameters);

  void initialize() override {}
  void finalize() override {}
  void execute() override;

  /**
   * Struct that stores all of the information about a PerfGraphNode,
   * which is used as the value for this Reporter
   */
  struct NodeEntry
  {
    std::string name;
    Real time;
    std::map<std::string, unsigned int> ints;
    std::vector<unsigned int> children_ids;
  };

private:
  /// Reporter value for the nodes in the graph
  std::vector<NodeEntry> & _nodes;
};

void to_json(nlohmann::json & json, const PerfGraphReporter::NodeEntry & entry);
void dataStore(std::ostream & stream, PerfGraphReporter::NodeEntry & entry, void * context);
void dataLoad(std::istream & stream, PerfGraphReporter::NodeEntry & entry, void * context);
