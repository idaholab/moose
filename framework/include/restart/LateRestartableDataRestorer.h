//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"

#include <typeinfo>
#include <string>

class RestartableDataReader;

/**
 * Restores late restartable data.
 *
 * This is a separate class so that we do not have to expose
 * all of RestartableDataReader.
 */
class LateRestartableDataRestorer
{
public:
  LateRestartableDataRestorer(RestartableDataReader & reader);

  /**
   * @return Whether or not data with the name \p name of type \p type on
   * thread \p tid is available for late restore
   */
  bool isRestorable(const std::string & name,
                    const std::type_info & type,
                    const THREAD_ID tid = 0) const;

  /**
   * @return Whether or not data with the name \p name and the type T
   * on thread \p tid is available for late restore
   */
  template <typename T>
  bool isRestorable(const std::string & name, const THREAD_ID tid = 0) const
  {
    return isRestorable(name, typeid(T), tid);
  }

  /**
   * Lately restores the value with name \p name on thread \p tid
   *
   * @return The value
   */
  template <typename T>
  const T & restore(const std::string & name, const THREAD_ID tid = 0);

private:
  /**
   * Internal method for restoring lately; needed due to header includes
   */
  const RestartableDataValue & restore(std::unique_ptr<RestartableDataValue> value,
                                       const THREAD_ID tid);

  /// The reader object
  RestartableDataReader & _reader;
};

template <typename T>
const T &
LateRestartableDataRestorer::restore(const std::string & name, const THREAD_ID tid /* = 0 */)
{
  static_assert(std::is_default_constructible_v<T>, "Must be default constructible");
  std::unique_ptr<RestartableDataValue> T_data =
      std::make_unique<RestartableData<T>>(name, nullptr);
  auto & value = restore(std::move(T_data), tid);
  auto T_value = dynamic_cast<const RestartableData<T> *>(&value);
  mooseAssert(T_value, "Bad cast");
  return T_value->get();
}
