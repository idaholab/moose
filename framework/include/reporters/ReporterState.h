//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

#include <iostream>

#include "libmesh/parallel.h"
#include "libmesh/parallel_object.h"

#include "ReporterName.h"
#include "RestartableData.h"
#include "ReporterMode.h"

/**
 * A special version of RestartableData to aid in storing Reporter values. This object is
 * used by the ReporterData object. The objects provides a convenient method to define
 * Reporter data that has a value as well as some number of old data values. Please refer to
 * ReporterData.h for more information regarding the use of this class.
 *
 * @param name The name of the Reporter value
 *
 * This class stores the old/older/... data using a std::list to allow for values to be inserted
 * into the data structure without corrupting references to the other values. This allows for
 * arbitrary time data to be stored.
 */
template <typename T>
class ReporterState : public RestartableData<std::pair<T, std::list<T>>>
{
public:
  ReporterState(const ReporterName & name);

  /**
   * Return the ReporterName that this state is associated
   */
  const ReporterName & getReporterName() const;

  ///@{
  /**
   * Return a reference to the current value or one of the old values.
   *
   * The time_index of 0 returns the current value, 1 returns old, 2 returns older, etc.
   */
  T & value(const std::size_t time_index = 0);
  const T & value(const std::size_t time_index = 0) const;
  ///@}

  /**
   * Add a mode that the value is consumed
   * @param mode The mode that the object will consume the Reporter value
   * @param object_name The name of the object doing the consuming (for error reporting)
   * @see ReporterData
   */
  void addConsumerMode(ReporterMode mode, const std::string & object_name);

  /**
   * Return the mode that the value is being consumed, see ReporterData
   * @see ReporterContext
   */
  const std::set<std::pair<ReporterMode, std::string>> & getConsumerModes() const;

  /**
   * Copy stored values back in time to old/older etc.
   */
  void copyValuesBack();

private:
  /// Name of data that state is associated
  const ReporterName _reporter_name;

  /// The mode(s) that the value is being consumed
  std::set<std::pair<ReporterMode, std::string>> _consumer_modes;
};

template <typename T>
ReporterState<T>::ReporterState(const ReporterName & name)
  : RestartableData<std::pair<T, std::list<T>>>(
        "ReporterData/" + name.getObjectName() + "/" + name.getValueName(), nullptr),
    _reporter_name(name)
{
}

template <typename T>
const ReporterName &
ReporterState<T>::getReporterName() const
{
  return _reporter_name;
}

template <typename T>
T &
ReporterState<T>::value(const std::size_t time_index)
{
  if (time_index == 0)
    return this->get().first;

  else if (time_index > this->get().second.size())
  {
    // Get the current oldest value from the list
    std::list<T> & old_values = this->get().second;
    const T & oldest = old_values.empty() ? this->get().first : old_values.back();

    // Add new elements to the current max size, filling with the oldest value that exists
    for (std::size_t i = old_values.size(); i < time_index; ++i)
      old_values.emplace_back(oldest);
  }

  // Return the desired value
  auto iter = this->get().second.begin();
  std::advance(iter, time_index - 1);
  return *(iter);
}

template <typename T>
const T &
ReporterState<T>::value(const std::size_t time_index) const
{
  if (time_index == 0)
    return this->get().first;
  else if (time_index > this->get().second.size())
    mooseError("The desired time index ",
               time_index,
               " does not exists for the '",
               _reporter_name,
               "' Reporter value, which contains ",
               this->get().second.size(),
               "old value(s). The getReporterValue method must be called with the desired time "
               "index to be able to access data.");

  auto iter = this->get().second.begin();
  std::advance(iter, time_index - 1);
  return *(iter);
}

template <typename T>
void
ReporterState<T>::addConsumerMode(ReporterMode mode, const std::string & object_name)
{
  mooseAssert(mode != REPORTER_MODE_UNSET, "UNSET cannot be used as the consumer mode");
  _consumer_modes.insert(std::make_pair(mode, object_name));
}

template <typename T>
const std::set<std::pair<ReporterMode, std::string>> &
ReporterState<T>::getConsumerModes() const
{
  return _consumer_modes;
}

template <typename T>
void
ReporterState<T>::copyValuesBack()
{
  // Ref. to the current
  T & value = this->set().first;
  std::list<T> & old_values = this->set().second;

  // Copy data back in time
  for (typename std::list<T>::reverse_iterator iter = old_values.rbegin();
       iter != old_values.rend();
       ++iter)
  {
    auto next_iter = std::next(iter, 1);
    if (next_iter == old_values.rend())
      (*iter) = value;
    else
      (*iter) = (*next_iter);
  }
}
