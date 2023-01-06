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

namespace moose::internal
{
class SolutionInvalidityRegistry;
class SolutionInvalidityInfo;
/**
 * Used to hold metadata about the registered sections
 * Note: this is a class instead of a struct because structs
 * are not able to be created in place using emplace_back in
 * C++11.  This will be fixed in C++20.
 */
class SolutionInvalidityInfo
{
public:
  SolutionInvalidityInfo(InvalidSolutionID id,
                         const std::string & name,
                         const std::string & message)
    : _id(id), _name(name), _message(message)
  {
  }

  /// Unique ID
  InvalidSolutionID _id;
  /// The name
  std::string _name;
  /// The message
  std::string _message;
};

/**
 * Get the global SolutionInvalidityRegistry singleton.
 */
SolutionInvalidityRegistry & getSolutionInvalidityRegistry();

/**
 * The place where all sections with solution invalid warnings will be stored
 */
class SolutionInvalidityRegistry : private GeneralRegistry<std::string, SolutionInvalidityInfo>
{
public:
  /**
   * Call to register an invalid calculation
   *
   * @param object_name The name of the object doing the registration
   * @param message The description of the solution invalid warning
   * @return The registered ID
   */
  InvalidSolutionID registerInvalidObject(const std::string & object_name,
                                          const std::string & message);

  /**
   * Given a name return the SolutionID
   * @object_name The name of the section
   * @return the ID
   */
  InvalidSolutionID sectionID(const std::string & object_name) const { return id(object_name); }

  /**
   * Given a SolutionID return the SolutionInvalidityInfo
   * @section_id The ID
   * @return The SolutionInvalidityInfo
   */
  const SolutionInvalidityInfo & sectionInfo(const InvalidSolutionID section_id) const
  {
    return item(section_id);
  }

  /**
   * Whether or not a section with that name has been registered
   * @object_name The name of the section
   * @return Whether or not it exists
   */
  bool sectionExists(const std::string & object_name) const { return keyExists(object_name); }

  /**
   * Whether or not a section with that id has been registered
   * @section_id The ID
   * @return Whether or not it exists
   */
  bool sectionExists(const InvalidSolutionID section_id) const { return idExists(section_id); }

  /**
   * @return number of registered sections
   */
  std::size_t numSections() const { return size(); }

private:
  SolutionInvalidityRegistry();

  /**
   * The internal function that actually carries out the registration
   */
  InvalidSolutionID actuallyRegisterSection(const std::string & object_name,
                                            const std::string & message);

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
   * @return the SolutionInvalidityInfo associated with the section_id
   */
  const SolutionInvalidityInfo & readSectionInfo(InvalidSolutionID section_id) const
  {
    return itemNonLocking(section_id);
  };

  /// So it can be constructed
  friend SolutionInvalidityRegistry & getSolutionInvalidityRegistry();
  /// This is only here so that SolutionInvalidity can access readSectionInfo
  friend SolutionInvalidity;
};

}
