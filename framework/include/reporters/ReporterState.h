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
 * This class stores the current/old/older/... data using a std::list to allow values to be inserted
 * into the data structure without corrupting references to the other values. This allows for
 * arbitrary time data to be stored.
 *
 * NOTE:
 * Access to the data should be through the value() method to ensure the data is allocated correctly
 */
template <typename T>
class ReporterState : public RestartableData<std::list<T>>
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

  /**
   * Load the data from stream (e.g., restart)
   *
   * This is a special version that handles the fact that the calls declare/getReporterValue
   * occur within the constructor of objects. As such, the storage list already contains data
   * and the references to this data must remain valid.
   *
   * The default dataLoad assumes the list being populated is empty and simply uses push_back.
   * Therefore, this function loads the data directly into the container to avoid this problem
   * and unnecessary copies.
   */
  virtual void load(std::istream & stream) override;

private:
  /// Name of data that state is associated
  const ReporterName _reporter_name;

  /// The mode(s) that the value is being consumed
  std::set<std::pair<ReporterMode, std::string>> _consumer_modes;
};

template <typename T>
ReporterState<T>::ReporterState(const ReporterName & name)
  : RestartableData<std::list<T>>(
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
  // Initialize the data; the first entry is the "current" data
  if (this->get().empty())
    this->set().resize(1);

  // Initialize old, older, ... data
  if (this->get().size() <= time_index)
    this->set().resize(time_index + 1, this->get().back());

  return *(std::next(this->get().begin(), time_index));
}

template <typename T>
const T &
ReporterState<T>::value(const std::size_t time_index) const
{
  if (this->get().size() <= time_index)
    mooseError("The desired time index ",
               time_index,
               " does not exists for the '",
               _reporter_name,
               "' Reporter value, which contains ",
               this->get().size(),
               " old value(s). The getReporterValue method must be called with the desired time "
               "index to be able to access data.");
  return *(std::next(this->get().begin(), time_index));
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
  std::list<T> & values = this->set();
  for (typename std::list<T>::reverse_iterator iter = values.rbegin();
       std::next(iter) != values.rend();
       ++iter)
    (*iter) = (*std::next(iter));
}

template <typename T>
void
ReporterState<T>::load(std::istream & stream)
{
  // Read the container size
  unsigned int size = 0;
  stream.read((char *)&size, sizeof(size));

  // If the current container is undersized, expand it to fit the loaded data
  if (this->get().size() < size)
    this->set().resize(size);

  // Load each entry of the list directly into the storage
  typename std::list<T>::iterator iter = this->set().begin();
  for (unsigned int i = 0; i < size; i++)
  {
    loadHelper(stream, *iter, nullptr);
    std::advance(iter, 1);
  }
}
