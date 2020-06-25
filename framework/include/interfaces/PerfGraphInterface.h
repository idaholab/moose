//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "PerfGuard.h"
#include "InputParameters.h"

#ifndef MOOSE_NO_PERF_GRAPH
#define TIME_SECTION(id) PerfGuard time_guard(_perf_graph, id);
#else
#define TIME_SECTION(id)
#endif

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
   * For objects that _are_ MooseObjects with a default prefix of type()
   */
  PerfGraphInterface(const MooseObject * moose_object);

  static InputParameters validParams();

  /**
   * For objects that _are_ MooseObjects
   */
  PerfGraphInterface(const MooseObject * moose_object, const std::string prefix);

  /**
   * For objects that aren't MooseObjects
   */
  PerfGraphInterface(PerfGraph & perf_graph, const std::string prefix = "");

  virtual ~PerfGraphInterface() = default;

protected:
  /**
   * Call to register a named section for timing.
   *
   * @param section_name The name of the code section to be timed
   * @param level The importance of the timer - lower is more important (0 will always come out)
   * @return The ID of the section - use when starting timing
   */
  PerfID registerTimedSection(const std::string & section_name, const unsigned int level);

  /// Params
  const InputParameters * const _pg_params;

  /// The performance graph to add to
  PerfGraph & _perf_graph;

  /// A prefix to use for all sections
  std::string _prefix;
};
