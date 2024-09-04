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
#include "MooseHashing.h"

#include <mutex>

// Forward Declarations
class SolutionInvalidity;
void dataStore(std::ostream &, SolutionInvalidity &, void *);

namespace moose::internal
{
class SolutionInvalidityRegistry;

/**
 * Get the global SolutionInvalidityRegistry singleton.
 */
SolutionInvalidityRegistry & getSolutionInvalidityRegistry();

/**
 * Helper class that stores the name associated with an invalid solution
 */
class SolutionInvalidityName
{
public:
  SolutionInvalidityName(const std::string & object_type, const std::string & message)
    : object_type(object_type), message(message)
  {
  }

  bool operator==(const SolutionInvalidityName & other) const
  {
    return object_type == other.object_type && message == other.message;
  }

  /// The type of the object
  std::string object_type;
  /// The invalid message
  std::string message;
};

/**
 * Helper class that hash the name associated with an invalid solution
 */
struct SoltionInvalidityNameHash
{
  inline size_t operator()(const SolutionInvalidityName & name) const
  {
    size_t seed = 0;
    Moose::hash_combine(seed, name.object_type, name.message);
    return seed;
  }
};

std::ostream & operator<<(std::ostream & os, const SolutionInvalidityName & name);

/**
 * Helper class that stores the info associated with an invalid solution
 */
class SolutionInvalidityInfo : public SolutionInvalidityName
{
public:
  SolutionInvalidityInfo(const std::string & object_type,
                         const std::string & message,
                         const InvalidSolutionID id,
                         const bool warning)
    : SolutionInvalidityName(object_type, message), id(id), warning(warning)
  {
  }

  /// The solution ID
  InvalidSolutionID id;
  /// Whether or not this is a warning
  bool warning;
};

/**
 * The place where all sections with solution invalid warnings will be stored
 */
class SolutionInvalidityRegistry : public GeneralRegistry<SolutionInvalidityName,
                                                          SolutionInvalidityInfo,
                                                          SoltionInvalidityNameHash>
{
public:
  /**
   * Call to register an invalid calculation
   *
   * @param object_type The type of the object doing the registration
   * @param message The description of the solution invalid warning
   * @param warning Whether or not it is a warning
   * @return The registered ID
   */
  InvalidSolutionID registerInvalidity(const std::string & object_type,
                                       const std::string & message,
                                       const bool warning);

private:
  SolutionInvalidityRegistry();

  /// So it can be constructed
  friend SolutionInvalidityRegistry & getSolutionInvalidityRegistry();
  /// This is only here so that SolutionInvalidity can access readSectionInfo
  friend SolutionInvalidity;
};
}
