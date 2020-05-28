//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RestartableData.h"
#include "ReporterState.h"
#include "libmesh/parallel_object.h"
#include "libmesh/auto_ptr.h"
#include "MooseApp.h"

/**
 * This is a helper class for managing the storage of declared Reporter object values. This design
 * of the system is a generalization of the VectorPostprocessor system that the Reporter objects
 * replaced.
 *
 * Foremost, this object doesn't store the data. It simply acts as helper for using the restart
 * system of the MooseApp. This object automatically handles the old, older, ... data. All
 * declarations create std::pair<T, std::vector<T>> restartable data on the MooseApp, where the
 * first value is the current value and the data in the vector are the older data.
 *
 * The ReporterState object is a RestartableData object that serves as a helper for managing the
 * time history it also includes a "context" object for preforming special operations. Refer to
 * ReporterState.h/C for more information.
 *
 * It is important to note that the Reporter values are not threaded. However, the Reporter
 * objects are UserObjects based, so the calculation of the values can be threaded.
 *
 * This object also relies on ReporterName objects, which are simply a combination of the
 * Reporter object name and the data name. If you recall the VectorPostprocessor system on which
 * this is based required an object name and a vector name. Th ReporterName class simply provides
 * a convenient way to provide that information in a single object. Special Parser syntax
 * was also defined so that application developers do not have to have input parameters for
 * both the object and data names (see Parser.C/h).
 */
class ReporterData
{
public:
  // The old/older values are stored in vector and this vector must have memory that doesn't
  // get reallocated. This is because the calls to getReporterValue can occur in any order using
  // any time index.
  constexpr static std::size_t HISTORY_CAPACITY = 5;

  ReporterData(MooseApp & moose_app);

  /**
   * Method for returning read only references to Reporter values.
   * @tparam T The Reporter value C++ type.
   * @param reporter_name The name of the reporter value, which includes the object name and the
   *                      data name.
   * @param time_index (optional) When not provided or zero is provided the current value is
   *                   returned. If an index greater than zero is provided then the corresponding
   *                   old data is returned (1 = old, 2 = older, etc.).
   */
  template <typename T>
  const T & getReporterValue(const ReporterName & reporter_name, const std::size_t time_index = 0);

  ///@{
  /**
   * Method for returning a writable reference to the current Reporter value. This method is
   * used by the Reporter class to produce values.
   * @tparam T The Reporter value C++ type.
   * @tparam S (optional) The Reporter context for performing specialized actions after the values
   *           have been computed. For example, ReporterBroadcastContext automatically broadcasts
   *           the computed value. See ReporterState.C/h for more information.
   * @param reporter_name The name of the reporter value, which includes the object name and the
   *                      data name.
   * @param default_value (optional )The value to assign to the current reporter value upon
   * creation.
   */
  template <typename T, template <typename> class S>
  T & declareReporterValue(const ReporterName & reporter_name);

  template <typename T, template <typename> class S>
  T & declareReporterValue(const ReporterName & reporter_name, const T & default_value);
  ///@}

  /**
   * Helper function for performing post calculation actions via the ReporterContext objects.
   *
   * If you recall, the original VectorPostprocessor system included the ability to perform some
   * scatter and broadcast actions via the special call on the storage helper object. This
   * is a replacement for that method that leverages the RepoorterContext objects to perform
   * value specific actions.
   *
   * See FEProblemBase::joinAndFinalize
   */
  void finalize(const std::string & object_name);

  /**
   * At the end of a timestep this method is called to copy the values back in time in preparation
   * for the next timestep.
   *
   * See FEProblemBase::advanceState
   */
  void copyValuesBack();

  /**
   * Copies the current value to all the desired old values and shrinks the old value vector to
   * the correct size based on the requested values.
   *
   * See InitReporterAction
   */
  void init();

private:
  /// For accessing the restart/recover system, which is where Reporter values are stored
  MooseApp & _app;

  /**
   * Helper object that for creating the necessary RestartableData for Reporter values.
   * @tparam T The desired C++ type for the Reporter value
   * @param reporter_name Object/data name for the Reporter value
   * @param declare Flag indicating if the ReporterValue is being declared or read. This flag
   *                is passed to the existing MooseApp restart/recover system that errors if a
   *                value is declared multiple times.
   */
  template <typename T>
  ReporterState<T> & getReporterStateHelper(const ReporterName & reporter_name, bool declare);

  /**
   * Helper object for declaring Reporter values.
   *
   * The ReporterContext objects allow for custom handling of data (e.g., broadcasting the value),
   * these context objects use the context system that exists on the MooseApp restart/recover
   * system. However, the get/declare methods can be called in any order thus an the underlying
   * RestartableData object is often created by the get method before it is declared. The get
   * methods should not have any knowledge of the ReporterContext object. As such the context object
   * must be applied to the RestartableData object after creation. This helper does just that.
   */
  template <typename T, template <typename> class S>
  ReporterState<T> & declareReporterValueHelper(const ReporterName & reporter_name);

  /// Convenience for accessing all the ReporterValues for a given Reporter object, the
  /// map key is the ReporterObject name. The data pointers are used by the init, finalize, and
  /// copyValuesBack methods. Those functions could operate using the MooseApp interface, but
  /// this is much more convient.
  std::unordered_map<std::string, std::set<RestartableDataValue *>> _data_ptrs;

  /// The ReporterContext objects are "attached" to the RestartableDataValue objects via a void
  /// pointer. Therefore, the actual object must be owned and managed by something, this is the
  /// something.
  std::set<std::unique_ptr<ReporterContextBase>> _context_ptrs;

  /// When true an error message is triggered so that the get/declare methods cannot be called
  /// outside of the object constructors.
  bool _initialized = false;
};

template <typename T>
ReporterState<T> &
ReporterData::getReporterStateHelper(const ReporterName & reporter_name, bool declare)
{
  // Avoid calling get/declare methods after object construction
  if (_initialized)
    mooseError("An attempt was made to declare or get Reporter data with the name '",
               reporter_name,
               "' after FEProblemBase::init(), calls to get or declare Reporter data cannot be "
               "made after FEProblem::init(); all calls should be made in the object constructor.");

  // Full name of the Reporter value to be stored in the MooseApp restart/recover system
  const std::string data_name =
      "ReporterData/" + reporter_name.getObjectName() + "/" + reporter_name.getValueName();

  // Creates the RestartableData object for storage in the MooseApp restart/recover system
  auto data_ptr = libmesh_make_unique<ReporterState<T>>(data_name);

  // Limits the number of old/older values. It is possible to call get
  data_ptr->get().second.resize(ReporterData::HISTORY_CAPACITY);
  RestartableDataValue & value =
      _app.registerRestartableData(data_name, std::move(data_ptr), 0, !declare);
  auto & data_ref = static_cast<ReporterState<T> &>(value);
  _data_ptrs[reporter_name.getObjectName()].insert(&data_ref);
  return data_ref;
}

template <typename T, template <typename> class S>
ReporterState<T> &
ReporterData::declareReporterValueHelper(const ReporterName & reporter_name)
{
  ReporterState<T> & data_ref = getReporterStateHelper<T>(reporter_name, true);

  // Create the ReporterContext object if it does not exist
  if (data_ref.context() == nullptr)
  {
    auto context_ptr = libmesh_make_unique<S<T>>(_app, data_ref);
    auto emplace_pair = _context_ptrs.emplace(std::move(context_ptr));
    data_ref.setContext(emplace_pair.first->get());
  }
  return data_ref;
}

template <typename T>
const T &
ReporterData::getReporterValue(const ReporterName & reporter_name, const std::size_t time_index)
{
  ReporterState<T> & data_ref = getReporterStateHelper<T>(reporter_name, false);
  return data_ref.value(time_index);
}

template <typename T, template <typename> class S>
T &
ReporterData::declareReporterValue(const ReporterName & reporter_name)
{
  ReporterState<T> & data_ref = declareReporterValueHelper<T, S>(reporter_name);
  return data_ref.value();
}

template <typename T, template <typename> class S>
T &
ReporterData::declareReporterValue(const ReporterName & reporter_name, const T & default_value)
{
  ReporterState<T> & data_ref = declareReporterValueHelper<T, S>(reporter_name);
  data_ref.get().first = default_value;
  return data_ref.value();
}
