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
#include "libmesh/simple_range.h"

#include "ReporterName.h"
#include "RestartableData.h"
#include "ReporterMode.h"

// Forward declarations
class MooseObject;
class ReporterContextBase;

/**
 * The base class for storing a Reporter's state
 *
 * The base class is needed in order to store the states without a template
 * parameter so that they can be iterated through to observe the producers
 * and consumers.
 */
class ReporterStateBase
{
public:
  ReporterStateBase(const ReporterName & name);
  virtual ~ReporterStateBase() = default;

  /**
   * Return the ReporterName that this state is associated with
   */
  const ReporterName & getReporterName() const { return _reporter_name; }

  /**
   * Add a consumer for this ReporterState
   * @param mode The mode that the object will consume the Reporter value
   * @param moose_object The MooseObject doing the consuming (for error reporting)
   * @see ReporterData
   */
  void addConsumer(ReporterMode mode, const MooseObject & moose_object);

  /**
   * Returns the consumers for this state; a pair that consists of the mode
   * that the state is being consumed by, and the object consuming it
   * @see ReporterContext
   */
  const std::set<std::pair<ReporterMode, const MooseObject *>> & getConsumers() const
  {
    return _consumers;
  }

  /**
   * @returns The type associated with this state
   */
  virtual std::string valueType() const = 0;

  /**
   * Sets the special Reporter type to a Postprocessor.
   *
   * See ReporterData::declareReporterValue.
   */
  void setIsPostprocessor() { _reporter_name.setIsPostprocessor(); }
  /**
   * Sets the special Reporter type to a VectorPostprocessor.
   *
   * See ReporterData::declareReporterValue.
   */
  void setIsVectorPostprocessor() { _reporter_name.setIsVectorPostprocessor(); }

private:
  /// Name of data that state is associated
  ReporterName _reporter_name;

  /// The consumers for this state; we store the MooseObject for detailed error reporting
  std::set<std::pair<ReporterMode, const MooseObject *>> _consumers;
};

/**
 * Custom sort for ReporterState::_consumers so that they are sorted by
 * object type, object name, and then mode, which makes for pretty output.
 */
bool operator<(const std::pair<ReporterMode, const MooseObject *> & a,
               const std::pair<ReporterMode, const MooseObject *> & b);

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
class ReporterState : public ReporterStateBase, public RestartableData<std::list<T>>
{
public:
  ReporterState(const ReporterName & name);

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
   * Copy stored values back in time to old/older etc.
   */
  void copyValuesBack();

  std::string valueType() const override final { return MooseUtils::prettyCppType<T>(); }

  /**
   * Loads and stores the data from/to a stream for restart
   *
   * This is a special version that handles the fact that the calls declare/getReporterValue
   * occur within the constructor of objects. As such, the storage list already contains data
   * and the references to this data must remain valid.
   *
   * The default dataLoad assumes the list being populated is empty and simply uses push_back.
   * Therefore, this function loads the data directly into the container to avoid this problem
   * and unnecessary copies.
   *
   * The default dataStore is very similar, but to ensure consistency (because we're re-defining
   * the load), we implement it again here.
   */
  ///@{
  void storeInternal(std::ostream & stream) override final;
  void loadInternal(std::istream & stream) override final;
  ///@}
};

template <typename T>
ReporterState<T>::ReporterState(const ReporterName & name)
  : ReporterStateBase(name), RestartableData<std::list<T>>(name.getRestartableName(), nullptr)
{
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

  return *(std::next(this->set().begin(), time_index));
}

template <typename T>
const T &
ReporterState<T>::value(const std::size_t time_index) const
{
  if (this->get().size() <= time_index)
    mooseError("The desired time index ",
               time_index,
               " does not exists for the '",
               getReporterName(),
               "' Reporter value, which contains ",
               this->get().size(),
               " old value(s). The getReporterValue method must be called with the desired time "
               "index to be able to access data.");
  return *(std::next(this->get().begin(), time_index));
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
ReporterState<T>::storeInternal(std::ostream & stream)
{
  // Store the container size
  std::size_t size = this->get().size();
  dataStore(stream, size, nullptr);

  // Store each entry of the list directly into the storage
  for (auto & val : this->set())
    storeHelper(stream, val, nullptr);
}

template <typename T>
void
ReporterState<T>::loadInternal(std::istream & stream)
{
  // Read the container size
  std::size_t size = 0;
  dataLoad(stream, size, nullptr);

  auto & values = this->set();

  // If the current container is undersized, expand it to fit the loaded data
  if (values.size() < size)
    values.resize(size);

  // Load each entry of the list directly into the storage
  // Because we don't shrink the container if the stored size is smaller than
  // our declared size, we have the odd iterator combo you see below
  for (auto & val : as_range(values.begin(), std::next(values.begin(), size)))
    loadHelper(stream, val, nullptr);
}
