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
#include "Restartable.h"

/**
 * A class which creates public interface functions for declaring and
 * getting restartable data. This is needed for objects which need to
 * store Restartable as a data member instead of inheriting from it.
 */
class PublicRestartable : public Restartable
{
public:
  /**
   * This class constructor is used for non-Moose-based objects like interfaces. A name for the
   * storage as well as a system name must be passed in along with the thread ID explicitly.
   * @param moose_app Reference to the application
   * @param name The name which is used when constructing the full-names of the restartable data.
   *             It is used with the following logic: `system_name/name/data_name`.
   *             (e.g. UserObjects/diffusion_kernel/coefficient). In most of the cases this is the
   *             name of the moose object.
   * @param system_name The name of the system where this object belongs to.
   * @param tid The thread ID.
   * @param read_only Switch to restrict the data for read-only.
   * @param metaname The name of the datamap where the restartable objects should be registered to.
   */
  PublicRestartable(MooseApp & moose_app,
                    const std::string & name,
                    const std::string & system_name,
                    THREAD_ID tid,
                    const bool read_only = false,
                    const RestartableDataMapName & metaname = "")
    : Restartable(moose_app, name, system_name, tid, read_only, metaname)
  {
  }

  /**
   * Declare a piece of data as "restartable" and initialize it.
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member
   * variable)
   * @param args Arguments to forward to the constructor of the data
   */
  template <typename T, typename... Args>
  T & declareRestartableData(const std::string & data_name, Args &&... args)
  {
    return Restartable::declareRestartableData<T>(data_name, std::forward<Args>(args)...);
  }

  /**
   * Declare a piece of data as "restartable" and initialize it
   * Similar to `declareRestartableData` but returns a const reference to the object.
   * Forwarded arguments are not allowed in this case because we assume that the
   * object is restarted and we won't need different constructors to initialize it.
   *
   * NOTE: This returns a _const reference_!  Make sure you store it in a _const reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   */
  template <typename T, typename... Args>
  const T & getRestartableData(const std::string & data_name) const
  {
    return Restartable::getRestartableData<T>(data_name);
  }
};
