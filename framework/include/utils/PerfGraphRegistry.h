//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralRegistry.h"
#include "MooseTypes.h"

#include <mutex>

// Forward Declarations
class PerfGraph;
class PerfGraphLivePrint;
class PerfNode;
void dataStore(std::ostream &, PerfGraph &, void *);

namespace moose
{
namespace internal
{
class PerfGraphRegistry;
class PerfGraphSectionInfo;
}
}

namespace moose
{
namespace internal
{

/**
 * Used to hold metadata about the registered sections
 * Note: this is a class instead of a struct because structs
 * are not able to be created in place using emplace_back in
 * C++11.  This will be fixed in C++20.
 */
class PerfGraphSectionInfo
{
public:
  PerfGraphSectionInfo() = default;
  PerfGraphSectionInfo(const PerfID id,
                       const std::string & name,
                       const unsigned int level,
                       const std::string & live_message,
                       const bool print_dots)
    : _id(id), _name(name), _level(level), _live_message(live_message), _print_dots(print_dots)
  {
  }

  /// Unique ID
  PerfID _id;

  /// The name
  std::string _name;

  /// Print level (verbosity level)
  unsigned int _level;

  /// Message to print while the section is running
  std::string _live_message;

  /// Whether or not to print dots while this section runs
  bool _print_dots;
};

/**
 * Get the global PerfGraphRegistry singleton.
 */
PerfGraphRegistry & getPerfGraphRegistry();

/**
 * The place where all timed sections will be stored
 */
class PerfGraphRegistry : private GeneralRegistry<std::string, PerfGraphSectionInfo>
{
public:
  /**
   * Call to register a named section for timing.
   *
   * @param section_name The name of the code section to be timed
   * @param level The importance of the timer - lower is more important (0 will always come out)
   * @return The ID of the section - use when starting timing
   */
  PerfID registerSection(const std::string & section_name, const unsigned int level);

  /**
   * Call to register a named section for timing.
   *
   * @param section_name The name of the code section to be timed
   * @param level The importance of the timer - lower is more important (0 will always come out)
   * @param live_message The message to be printed to the screen during execution
   * @param print_dots Whether or not progress dots should be printed for this section
   * @return The ID of the section - use when starting timing
   */
  PerfID registerSection(const std::string & section_name,
                         const unsigned int level,
                         const std::string & live_message,
                         const bool print_dots = true);

  /**
   * Given a name return the PerfID
   * @section_name The name of the section
   * @return the ID
   */
  PerfID sectionID(const std::string & section_name) const { return id(section_name); }

  /**
   * Given a PerfID return the PerfGraphSectionInfo
   * @section_id The ID
   * @return The PerfGraphSectionInfo
   */
  const PerfGraphSectionInfo & sectionInfo(const PerfID section_id) const
  {
    return item(section_id);
  }

  /**
   * Whether or not a section with that name has been registered
   * @section_name The name of the section
   * @return Whether or not it exists
   */
  bool sectionExists(const std::string & section_name) const { return keyExists(section_name); }

  /**
   * Whether or not a section with that id has been registered
   * @section_id The ID
   * @return Whether or not it exists
   */
  bool sectionExists(const PerfID section_id) const { return idExists(section_id); }

  /**
   * @return number of registered sections
   */
  std::size_t numSections() const { return size(); }

private:
  PerfGraphRegistry();

  /**
   * The internal function that actually carries out the registration
   */
  PerfID actuallyRegisterSection(const std::string & section_name,
                                 const unsigned int level,
                                 const std::string & live_message,
                                 const bool print_dots = true);

  /**
   * Special accessor just for PerfGraph so that
   * no locking is needed in PerfGraph.  This could
   * probably be removed once we have C++17 with shared_mutex
   *
   * This function is NOT threadsafe - but it is ok
   * for PerfGraph to call it because only the main
   * thread will be registering sections and only
   * the main thread will be running PerfGraph routines
   *
   * @return the PerfGraphSectionInfo associated with the section_id
   */
  const PerfGraphSectionInfo & readSectionInfo(PerfID section_id) const
  {
    return itemNonLocking(section_id);
  };

  /// So it can be constructed
  friend PerfGraphRegistry & getPerfGraphRegistry();
  /// This is only here so that PerfGraph can access readSectionInfo
  friend PerfGraph;
  // For accessing _id_to_section_info when storing the PerfGraph
  friend void ::dataStore(std::ostream &, PerfGraph &, void *);
};

}
}

void dataStore(std::ostream & stream, moose::internal::PerfGraphSectionInfo & info, void * context);
void dataLoad(std::istream & stream, moose::internal::PerfGraphSectionInfo & info, void * context);
