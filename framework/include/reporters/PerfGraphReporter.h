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

class PerfNode;
class PerfGraph;

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
  void execute() override {}

private:
  const PerfGraph * const & _graph;
};

void to_json(nlohmann::json & json, const PerfGraph * const & perf_graph);
void to_json(nlohmann::json & json, const PerfNode & node);

/**
 * Store and load methods for const PerfGraph *, used in the PerfGraphReporter,
 * which does nothing.
 *
 * They do nothing because the recover capability is retained in the
 * PerfGraph itself, which will recover and append a previously ran
 * graph to the current PerfGraph.
 */
///@{
void
dataStore(std::ostream &, const PerfGraph *&, void *)
{
}
void
dataLoad(std::istream &, const PerfGraph *&, void *)
{
}
///@}
