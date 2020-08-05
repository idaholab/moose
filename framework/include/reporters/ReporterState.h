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

/**
 * A special version of RestartableData to aid in storing Reporter values. This object is
 * used by the ReporterData object. The objects provides a convenient method to define
 * Reporter data that has a value as well as some number of old data values. Please refer to
 * ReporterData.h for more information regarding the use of this class.
 */
template <typename T>
class ReporterState : public RestartableData<std::pair<T, std::vector<T>>>
{
public:
  ReporterState(const ReporterName & name);

  /**
   * Return the ReporterName that this state is associated
   */
  const ReporterName & getReporterName() const;

  /**
   * Return a reference to the current value or one of the old values.
   *
   * The time_index of 0 returns the current value, 1 returns old, 2 returns older, etc.
   */
  T & value(const std::size_t time_index = 0);

  /**
   * This object tracks the max requested index, which is used by the ReporterContext to manage
   * the old values
   */
  std::size_t getMaxRequestedTimeIndex() const;

private:
  /// Tracking of the largest desired old value
  std::size_t _max_requested_time_index = 0;

  /// Name of data that state is associated
  const ReporterName _reporter_name;
};

template <typename T>
ReporterState<T>::ReporterState(const ReporterName & name)
  : RestartableData<std::pair<T, std::vector<T>>>(
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
  _max_requested_time_index = std::max(_max_requested_time_index, time_index);
  if (time_index == 0)
    return this->get().first;
  else
  {
    mooseAssert(time_index - 1 < this->get().second.size(),
                "The desired time index " << time_index << " is out of range of the size of "
                                          << this->get().second.size());
    return this->get().second[time_index - 1];
  }
}

template <typename T>
std::size_t
ReporterState<T>::getMaxRequestedTimeIndex() const
{
  return _max_requested_time_index;
}
