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
class SolutionInvalidity;
void dataStore(std::ostream &, SolutionInvalidity &, void *);

namespace moose
{
namespace internal
{
class SolutionInvalidityRegistry;
class SolutionInvaliditySectionInfo;
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
class SolutionInvaliditySectionInfo
{
public:
  SolutionInvaliditySectionInfo(SolutionID id, std::string name) : _id(id), _name(name) {}

  /// Unique ID
  SolutionID _id;
  /// The name
  std::string _name;
};

/**
 * Get the global SolutionInvalidityRegistry singleton.
 */
SolutionInvalidityRegistry & getSolutionInvalidityRegistry();

/**
 * The place where all sections with solution invalid warnings will be stored
 */
class SolutionInvalidityRegistry
  : private GeneralRegistry<std::string, SolutionInvaliditySectionInfo>
{
public:
  /**
   * Call to register a named section for detecting solution invalid.
   *
   * @param section_name The name of the code section to be detected
   * @return The ID of the section - use when counting solution invalid warning
   */
  SolutionID registerSection(const std::string & section_name);

  /**
   * Given a name return the SolutionID
   * @section_name The name of the section
   * @return the ID
   */
  SolutionID sectionID(const std::string & section_name) const { return id(section_name); }

  /**
   * Given a SolutionID return the SolutionInvaliditySectionInfo
   * @section_id The ID
   * @return The SolutionInvaliditySectionInfo
   */
  const SolutionInvaliditySectionInfo & sectionInfo(const SolutionID section_id) const
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
  bool sectionExists(const SolutionID section_id) const { return idExists(section_id); }

  /**
   * @return number of registered sections
   */
  std::size_t numSections() const { return size(); }

private:
  SolutionInvalidityRegistry();

  /**
   * The internal function that actually carries out the registration
   */
  SolutionID actuallyRegisterSection(const std::string & section_name);

  /**
   * Special accessor just for SolutionInvalidity so that
   * no locking is needed in SolutionInvalidity.  This could
   * probably be removed once we have C++17 with shared_mutex
   *
   * This function is NOT threadsafe - but it is ok
   * for SolutionInvalidity to call it because only the main
   * thread will be registering sections and only
   * the main thread will be running SolutionInvalidity routines
   *
   * @return the SolutionInvaliditySectionInfo associated with the section_id
   */
  const SolutionInvaliditySectionInfo & readSectionInfo(SolutionID section_id) const
  {
    return itemNonLocking(section_id);
  };

  /// So it can be constructed
  friend SolutionInvalidityRegistry & getSolutionInvalidityRegistry();
  /// This is only here so that SolutionInvalidity can access readSectionInfo
  friend SolutionInvalidity;
  // For accessing _id_to_section_info when storing the SolutionInvalidity
  friend void ::dataStore(std::ostream &, SolutionInvalidity &, void *);
};

}
}

void dataStore(std::ostream & stream,
               moose::internal::SolutionInvaliditySectionInfo & info,
               void * context);
void dataLoad(std::istream & stream,
              moose::internal::SolutionInvaliditySectionInfo & info,
              void * context);
