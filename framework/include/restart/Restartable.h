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

#define adDeclareRestartableData                                                                   \
  _Pragma("GCC warning \"adDeclareRestartableData is deprecated. Simply use "                      \
          "declareRestartableData\"") this->template declareRestartableDataTempl

#define declareRestartableData this->template declareRestartableDataTempl

// Forward declarations
class PostprocessorData;
class SubProblem;
class InputParameters;
class MooseObject;
class MooseApp;

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
   */
  Restartable(MooseApp & moose_app,
              const std::string & name,
              const std::string & system_name,
              THREAD_ID tid);

  /**
   * Emtpy destructor
   */
  virtual ~Restartable() = default;

protected:
  /**
   * Declare a piece of data as "restartable".
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   */
  template <typename T>
  T & declareRestartableDataTempl(std::string data_name);

  /**
   * Declare a piece of data as "restartable" and initialize it.
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param init_value The initial value of the data
   */
  template <typename T>
  T & declareRestartableDataTempl(std::string data_name, const T & init_value);

  /**
   * Declare a piece of data as "restartable".
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param context Context pointer that will be passed to the load and store functions
   */
  template <typename T>
  T & declareRestartableDataWithContext(std::string data_name, void * context);

  /**
   * Declare a piece of data as "restartable" and initialize it.
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param init_value The initial value of the data
   * @param context Context pointer that will be passed to the load and store functions
   */
  template <typename T>
  T &
  declareRestartableDataWithContext(std::string data_name, const T & init_value, void * context);

  /**
   * Declare a piece of data as "recoverable".
   * This means that in the event of a recovery this piece of data
   * will be restored back to its previous value.
   *
   * Note - this data will NOT be restored on _Restart_!
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   */
  template <typename T>
  T & declareRecoverableData(std::string data_name);

  /**
   * Declare a piece of data as "restartable" and initialize it.
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * Note - this data will NOT be restored on _Restart_!
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param init_value The initial value of the data
   */
  template <typename T>
  T & declareRecoverableData(std::string data_name, const T & init_value);

  /**
   * Declare a piece of data as "restartable".
   * This means that in the event of a restart this piece of data
   * will be restored back to its previous value.
   *
   * NOTE: This returns a _reference_!  Make sure you store it in a _reference_!
   *
   * @param data_name The name of the data (usually just use the same name as the member variable)
   * @param object_name A supplied name for the object that is declaring this data.
   */
  template <typename T>
  T & declareRestartableDataWithObjectName(std::string data_name, std::string object_name);

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
   */
  template <typename T>
  T & declareRestartableDataWithObjectNameWithContext(std::string data_name,
                                                      std::string object_name,
                                                      void * context);

private:
  /// Helper function for actually registering the restartable data.
  void registerRestartableDataOnApp(std::string name,
                                    std::unique_ptr<RestartableDataValue> data,
                                    THREAD_ID tid);

  /// Helper function for actually registering the restartable data.
  void registerRecoverableDataOnApp(std::string name);

  /// Reference to the application
  MooseApp & _restartable_app;

  /// The name of the object
  std::string _restartable_name;

  /// The system name this object is in
  std::string _restartable_system_name;

  /// The thread ID for this object
  THREAD_ID _restartable_tid;
};

template <typename T>
T &
Restartable::declareRestartableDataTempl(std::string data_name)
{
  return declareRestartableDataWithContext<T>(data_name, nullptr);
}

template <typename T>
T &
Restartable::declareRestartableDataTempl(std::string data_name, const T & init_value)
{
  return declareRestartableDataWithContext<T>(data_name, init_value, nullptr);
}

template <typename T>
T &
Restartable::declareRestartableDataWithContext(std::string data_name, void * context)
{
  std::string full_name = _restartable_system_name + "/" + _restartable_name + "/" + data_name;
  auto data_ptr = libmesh_make_unique<RestartableData<T>>(full_name, context);
  T & restartable_data_ref = data_ptr->get();

  registerRestartableDataOnApp(full_name, std::move(data_ptr), _restartable_tid);

  return restartable_data_ref;
}

template <typename T>
T &
Restartable::declareRestartableDataWithContext(std::string data_name,
                                               const T & init_value,
                                               void * context)
{
  std::string full_name = _restartable_system_name + "/" + _restartable_name + "/" + data_name;
  auto data_ptr = libmesh_make_unique<RestartableData<T>>(full_name, context);
  data_ptr->set() = init_value;

  T & restartable_data_ref = data_ptr->get();
  registerRestartableDataOnApp(full_name, std::move(data_ptr), _restartable_tid);

  return restartable_data_ref;
}

template <typename T>
T &
Restartable::declareRestartableDataWithObjectName(std::string data_name, std::string object_name)
{
  return declareRestartableDataWithObjectNameWithContext<T>(data_name, object_name, nullptr);
}

template <typename T>
T &
Restartable::declareRestartableDataWithObjectNameWithContext(std::string data_name,
                                                             std::string object_name,
                                                             void * context)
{
  std::string old_name = _restartable_name;

  _restartable_name = object_name;

  T & value = declareRestartableDataWithContext<T>(data_name, context);

  _restartable_name = old_name;

  return value;
}

template <typename T>
T &
Restartable::declareRecoverableData(std::string data_name)
{
  std::string full_name = _restartable_system_name + "/" + _restartable_name + "/" + data_name;

  registerRecoverableDataOnApp(full_name);

  return declareRestartableDataWithContext<T>(data_name, nullptr);
}

template <typename T>
T &
Restartable::declareRecoverableData(std::string data_name, const T & init_value)
{
  std::string full_name = _restartable_system_name + "/" + _restartable_name + "/" + data_name;

  registerRecoverableDataOnApp(full_name);

  return declareRestartableDataWithContext<T>(data_name, init_value, nullptr);
}
