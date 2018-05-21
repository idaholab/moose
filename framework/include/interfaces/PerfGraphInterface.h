//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PERFGRAPHINTERFACE_H
#define PERFGRAPHINTERFACE_H

#include "Moose.h"
#include "PerfGuard.h"

#define TIME_SECTION(id) PerfGuard time_guard(_perf_graph, id);

// Forward declarations
class PerfGraphInterface;

template <>
InputParameters validParams<PerfGraphInterface>();

/**
 * Interface for objects that needs transient capabilities
 */
class PerfGraphInterface
{
public:
  /**
   * For objects that _are_ MooseObjects
   */
  PerfGraphInterface(const MooseObject * moose_object, std::string prefix = "");

  /**
   * For objects that aren't MooseObjects
   */
  PerfGraphInterface(PerfGraph & perf_graph, std::string prefix = "");

  virtual ~PerfGraphInterface();

protected:
  /**
   * Call to register a named section for timing.
   *
   * @return The ID of the section - use when starting timing
   */
  PerfID registerTimedSection(std::string section_name);

  /// Params
  const InputParameters * _pg_params;

  /// The performance graph to add to
  PerfGraph & _perf_graph;

  /// A prefix to use for all sections
  std::string _prefix;
};

#endif /* PERFGRAPHINTERFACE_H */
