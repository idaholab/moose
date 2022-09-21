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

#ifndef MOOSE_NO_PERF_GRAPH
#define TIME_SECTION1(id)                                                                          \
  mooseAssert(!Threads::in_threads, "PerfGraph timing cannot be used within threaded sections");   \
  PerfGuard time_guard(this->_pg_moose_app.perfGraph(), id);
#else
#define TIME_SECTION1(id)
#endif

#define TIME_SECTION2(section_name, level)                                                         \
  static const PerfID __perf_id = this->registerTimedSection(section_name, level);                 \
  TIME_SECTION1(__perf_id);

#define TIME_SECTION3(section_name, level, live_message)                                           \
  static const PerfID __perf_id = this->registerTimedSection(section_name, level, live_message);   \
  TIME_SECTION1(__perf_id);

#define TIME_SECTION4(section_name, level, live_message, print_dots)                               \
  static const PerfID __perf_id =                                                                  \
      this->registerTimedSection(section_name, level, live_message, print_dots);                   \
  TIME_SECTION1(__perf_id);

// Overloading solution from https://stackoverflow.com/a/11763277
#define GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define TIME_SECTION(...)                                                                          \
  GET_MACRO(__VA_ARGS__, TIME_SECTION4, TIME_SECTION3, TIME_SECTION2, TIME_SECTION1, )(__VA_ARGS__)

class InputParameters;
class MooseObject;

/**
 * Interface for objects interacting with the PerfGraph.
 *
 * Enables getting PerfGraph information and registering PerfGraph timed sections.
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

  /**
   * For objects that construct the PerfGraphInterface _before_ the PerfGraph
   * is initialized (see MooseApp and OutputWarehouse)
   */
  PerfGraphInterface(MooseApp & moose_app, const std::string prefix = "");

  virtual ~PerfGraphInterface() = default;

  /**
   * Get the PerfGraph
   */
  PerfGraph & perfGraph();

protected:
  /**
   * Call to register a named section for timing.
   *
   * @param section_name The name of the code section to be timed
   * @param level The importance of the timer - lower is more important (0 will always come out)
   * @return The ID of the section - use when starting timing
   */
  PerfID registerTimedSection(const std::string & section_name, const unsigned int level) const;

  /**
   * Call to register a named section for timing.
   *
   * @param section_name The name of the code section to be timed
   * @param level The importance of the timer - lower is more important (0 will always come out)
   * @param live_message The message to be printed to the screen during execution
   * @param print_dots Whether or not progress dots should be printed for this section
   * @return The ID of the section - use when starting timing
   */
  PerfID registerTimedSection(const std::string & section_name,
                              const unsigned int level,
                              const std::string & live_message,
                              const bool print_dots = true) const;

  /// The MooseApp that owns the PerfGraph
  MooseApp & _pg_moose_app;

  /// A prefix to use for all sections
  const std::string _prefix;
};
