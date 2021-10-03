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
};

void to_json(nlohmann::json & json, const PerfGraph * const & perf_graph);
/**
 * Data store and load methods for PerfGraph, which do nothing.
 */
///@{
void dataStore(std::ostream & stream, const PerfGraph *& perf_graph, void * context);
void dataLoad(std::istream & stream, const PerfGraph *& perf_graph, void * context);
///@}
