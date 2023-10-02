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
   * Restores the declared data with the name \p name on thread \p tid
   */
  void restore(const std::string & name, const THREAD_ID tid = 0);

private:
  /**
   * Internal method for restoring lately; needed due to header includes
   */
  const RestartableDataValue & restore(std::unique_ptr<RestartableDataValue> value,
                                       const THREAD_ID tid);

  /// The reader object
  RestartableDataReader & _reader;
};
