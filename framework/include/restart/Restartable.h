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
#include "MooseTypes.h"
#include "RestartableData.h"

// Forward declarations
class PostprocessorData;
class SubProblem;
class InputParameters;
class MooseObject;
class MooseApp;
class MooseMesh;

/**
 * A class for creating restricted objects
 * \see BlockRestartable BoundaryRestartable
 */
class Restartable
{
public:
  /**
   * Class constructor
   *
   * @param moose_object The MooseObject that this interface is being implemented on.
   * @param system_name The name of the MOOSE system.  ie "Kernel", "BCs", etc.  Should roughly
   * correspond to the section in the input file so errors are easy to understand.
   *
   * This method will forward the thread id if it exists in the moose_object parameters. Delegates
   * to the "MooseApp &" constructor.
   */
  Restartable(const MooseObject * moose_object, const std::string & system_name);

  /**
   * Class constructor
   *
   * Similar to the other class constructor but also accepts an individual thread ID. If this
   * method is used, no thread ID in the parameters object is used. Delegates to the "MooseApp &"
   * constructor.
   */
  Restartable(const MooseObject * moose_object, const std::string & system_name, THREAD_ID tid);

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
  Restartable(MooseApp & moose_app,
              const std::string & name,
              const std::string & system_name,
              THREAD_ID tid,
              const bool read_only = false,
              const RestartableDataMapName & metaname = "");

protected:
  /**
   * Declare a piece of data as "restartable" and initialize it.
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param args Arguments to forward to the constructor of the data
   */
  template <typename T, typename... Args>
  T & declareRestartableData(const std::string & data_name, Args &&... args);

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
  const T & getRestartableData(const std::string & data_name) const;

  /**
   * Declare a piece of data as "restartable" and initialize it.
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param context Context pointer that will be passed to the load and store functions
   * @param args Arguments to forward to the constructor of the data
   */
  template <typename T, typename... Args>
  T &
  declareRestartableDataWithContext(const std::string & data_name, void * context, Args &&... args);

  /**
   * Declare a piece of data as "recoverable" and initialize it.
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * Note - this data will NOT be restored on _Restart_!
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param args Arguments to forward to the constructor of the data
   */
  template <typename T, typename... Args>
  T & declareRecoverableData(const std::string & data_name, Args &&... args);

  /**
   * Declare a piece of data as "restartable".
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param object_name A supplied name for the object that is declaring this data.
   * @param args Arguments to forward to the constructor of the data
   */
  template <typename T, typename... Args>
  T & declareRestartableDataWithObjectName(const std::string & data_name,
                                           const std::string & object_name,
                                           Args &&... args);

  /**
   * Declare a piece of data as "restartable".
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param object_name A supplied name for the object that is declaring this data.
   * @param context Context pointer that will be passed to the load and store functions
   * @param args Arguments to forward to the constructor of the data
   */
  template <typename T, typename... Args>
  T & declareRestartableDataWithObjectNameWithContext(const std::string & data_name,
                                                      const std::string & object_name,
                                                      void * context,
                                                      Args &&... args);

  /**
   * Gets the name of a piece of restartable data given a data name, adding
   * the system name and object name prefix.
   *
   * This should only be used in this interface and in testing.
   */
  std::string restartableName(const std::string & data_name) const;

  /// Reference to the application
  MooseApp & _restartable_app;

  /// The system name this object is in
  const std::string _restartable_system_name;

  /// The thread ID for this object
  const THREAD_ID _restartable_tid;

  /// Flag for toggling read only status (see ReporterData)
  const bool _restartable_read_only;

private:
  /// Restartable metadata name
  const RestartableDataMapName _metaname;

  /// The name of the object
  std::string _restartable_name;

  /// Helper function for actually registering the restartable data.
  RestartableDataValue & registerRestartableDataOnApp(const std::string & name,
                                                      std::unique_ptr<RestartableDataValue> data,
                                                      THREAD_ID tid) const;

  /// Helper function for actually registering the restartable data.
  void registerRestartableNameWithFilterOnApp(const std::string & name,
                                              Moose::RESTARTABLE_FILTER filter);

  /**
   * Helper function for declaring restartable data. We use this function to reduce code duplication
   * when returning const/nonconst references to the data.
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param context Context pointer that will be passed to the load and store functions
   * @param args Arguments to forward to the constructor of the data
   */
  template <typename T, typename... Args>
  RestartableData<T> & declareRestartableDataHelper(const std::string & data_name,
                                                    void * context,
                                                    Args &&... args) const;
};

template <typename T, typename... Args>
T &
Restartable::declareRestartableData(const std::string & data_name, Args &&... args)
{
  return declareRestartableDataWithContext<T>(data_name, nullptr, std::forward<Args>(args)...);
}

template <typename T, typename... Args>
const T &
Restartable::getRestartableData(const std::string & data_name) const
{
  return declareRestartableDataHelper<T>(data_name, nullptr).get();
}

template <typename T, typename... Args>
T &
Restartable::declareRestartableDataWithContext(const std::string & data_name,
                                               void * context,
                                               Args &&... args)
{
  return declareRestartableDataHelper<T>(data_name, context, std::forward<Args>(args)...).set();
}

template <typename T, typename... Args>
RestartableData<T> &
Restartable::declareRestartableDataHelper(const std::string & data_name,
                                          void * context,
                                          Args &&... args) const
{
  const auto full_name = restartableName(data_name);

  // Here we will create the RestartableData even though we may not use this instance.
  // If it's already in use, the App will return a reference to the existing instance and we'll
  // return that one instead. We might refactor this to have the app create the RestartableData
  // at a later date.
  auto data_ptr =
      std::make_unique<RestartableData<T>>(full_name, context, std::forward<Args>(args)...);
  auto & restartable_data_ref = static_cast<RestartableData<T> &>(
      registerRestartableDataOnApp(full_name, std::move(data_ptr), _restartable_tid));

  return restartable_data_ref;
}

template <typename T, typename... Args>
T &
Restartable::declareRestartableDataWithObjectName(const std::string & data_name,
                                                  const std::string & object_name,
                                                  Args &&... args)
{
  return declareRestartableDataWithObjectNameWithContext<T>(
      data_name, object_name, nullptr, std::forward<Args>(args)...);
}

template <typename T, typename... Args>
T &
Restartable::declareRestartableDataWithObjectNameWithContext(const std::string & data_name,
                                                             const std::string & object_name,
                                                             void * context,
                                                             Args &&... args)
{
  std::string old_name = _restartable_name;

  _restartable_name = object_name;

  T & value = declareRestartableDataWithContext<T>(data_name, context, std::forward<Args>(args)...);

  _restartable_name = old_name;

  return value;
}

template <typename T, typename... Args>
T &
Restartable::declareRecoverableData(const std::string & data_name, Args &&... args)
{
  const auto full_name = restartableName(data_name);

  registerRestartableNameWithFilterOnApp(full_name, Moose::RESTARTABLE_FILTER::RECOVERABLE);

  return declareRestartableDataWithContext<T>(data_name, nullptr, std::forward<Args>(args)...);
}
