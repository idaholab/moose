//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MultiMooseEnum.h"

/**
 * A MultiMooseEnum object to hold "execute_on" flags.
 *
 * This object allows available flags to be added or removed thus each object can control the
 * flags that are available.
 */
class ExecFlagEnum : public MultiMooseEnum
{
public:
  ExecFlagEnum();
  ExecFlagEnum(const ExecFlagEnum & other);
  ExecFlagEnum(const MultiMooseEnum & other);
  ExecFlagEnum & operator=(const ExecFlagEnum & other) = default;

  ///@{
  /**
   * Add additional execute_on flags to the list of possible flags.
   *
   * Use a recursive variadic template function to allow for an arbitrary
   * number arguments:
   *     addAvaiableFlags(EXEC_INITIAL);
   *     addAvaiableFlags(EXEC_INITIAL, EXEC_TIMESTEP_END);
   */
  template <typename... Args>
  void addAvailableFlags(const ExecFlagType & flag, Args... flags);
  void addAvailableFlags(const ExecFlagType & flag);
  ///@}

  ///@{
  /**
   * Assignment operators for setting the current flags.
   */
  using MultiMooseEnum::operator=; // use parent methods
  ExecFlagEnum & operator=(const std::initializer_list<ExecFlagType> & flags);
  ExecFlagEnum & operator=(const ExecFlagType & flags);
  ExecFlagEnum & operator+=(const std::initializer_list<ExecFlagType> & flags);
  ExecFlagEnum & operator+=(const ExecFlagType & flags);
  ///@}

  ///@{
  /**
   * Remove flags from being available.
   */
  template <typename... Args>
  void removeAvailableFlags(const ExecFlagType & flag, Args... flags);
  void removeAvailableFlags(const ExecFlagType & flag);
  ///@}

  /**
   * Generate a documentation string for the "execute_on" parameter.
   */
  std::string getDocString() const;

  /**
   * Reference the all the available items.
   */
  const std::set<ExecFlagType> & items() const { return _items; }

protected:
  /**
   *  Append the list of current flags.
   */
  void appendCurrent(const ExecFlagType & item);
};

template <typename... Args>
void
ExecFlagEnum::addAvailableFlags(const ExecFlagType & flag, Args... flags)
{
  addAvailableFlags(flag);
  addAvailableFlags(flags...);
}

template <typename... Args>
void
ExecFlagEnum::removeAvailableFlags(const ExecFlagType & flag, Args... flags)
{
  removeAvailableFlags(flag);
  removeAvailableFlags(flags...);
}
