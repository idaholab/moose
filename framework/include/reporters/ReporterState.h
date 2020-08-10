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
 *
 * @param name The name of the Reporter value
 *
 * NOTE:
 * From a pure design point of view the init/copyValuesBack methods of ReporterContext would be
 * within this class and the ReporterContext would just be a helper for parallel computation
 * on the producer side. However, the ReporterState must be of type RestartableData because it
 * is the object stored within MooseApp restart data structures. As such, it is not easy for this
 * class to have a non-templated base class that can be stored in ReporterData. Therefore,
 * these functions are located in ReporterContext. There is additional information regarding this
 * design choice in the ReporterData object comments.
 *
 * The producer/consumer modes must be stored on this object rather than the ReporterContext
 * for the same as above, since this state is created by either the get or declare calls.
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

  /**
   * Set the mode that the value is produced
   * @see ReporterData
   *
   * The setter is needed because the producer mode cannot be set in the constructor, because this
   * state object can be first created by either the consumer or producer.
   */
  void setProducerMode(Moose::ReporterMode mode);

  /**
   * Return the mode that the Reporter value is being produced.
   * @see ReporterContext
   */
  Moose::ReporterMode getProducerMode() const;

  /**
   * Add a mode that the value is consumed
   * @param mode The mode that the object will consume the Reporter value
   * @param object_name The name of the object doing the consuming (for error reporting)
   * @see ReporterData
   */
  void addConsumerMode(Moose::ReporterMode mode, const std::string & object_name);

  /**
   * Return the mode that the value is being consumed, see ReporterData
   * @see ReporterContext
   */
  const std::set<std::pair<Moose::ReporterMode, std::string>> & getConsumerModes() const;

private:
  // This is mutable because the value() method increments it, but in practice th value() method
  // should be const, since retrieving the data should not change it.
  /// Tracking of the largest desired old value
  mutable std::size_t _max_requested_time_index = 0;

  /// Name of data that state is associated
  const ReporterName _reporter_name;

  /// The mode that the value is being produced
  Moose::ReporterMode _producer_mode = Moose::ReporterMode::UNSET;

  /// The mode(s) that the value is being consumed
  std::set<std::pair<Moose::ReporterMode, std::string>> _consumer_modes;
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

template <typename T>
Moose::ReporterMode
ReporterState<T>::getProducerMode() const
{
  return _producer_mode;
}

template <typename T>
void
ReporterState<T>::setProducerMode(Moose::ReporterMode mode)
{
  _producer_mode = mode;
}

template <typename T>
void
ReporterState<T>::addConsumerMode(Moose::ReporterMode mode, const std::string & object_name)
{
  mooseAssert(mode != Moose::ReporterMode::UNSET, "UNSET cannot be used as the consumer mode");
  _consumer_modes.insert(std::make_pair(mode, object_name));
}

template <typename T>
const std::set<std::pair<Moose::ReporterMode, std::string>> &
ReporterState<T>::getConsumerModes() const
{
  return _consumer_modes;
}
