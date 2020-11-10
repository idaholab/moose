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
#include "ReporterContext.h"
#include "libmesh/parallel_object.h"
#include "libmesh/auto_ptr.h"

class MooseApp;

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
 * time history. A "context" object also exists that uses the ReporterState for preforming special
 * operations. Refer to ReporterState.h/C for more information.
 *
 * It is important to note that the Reporter values are not threaded. However, the Reporter
 * objects are UserObject based, so the calculation of the values can be threaded.
 *
 * This object also relies on ReporterName objects, which are simply a combination of the
 * Reporter object name and the data name. If you recall the VectorPostprocessor system on which
 * this is based required an object name and a vector name. The ReporterName class simply provides
 * a convenient way to provide that information in a single object. Special Parser syntax
 * was also defined so that application developers do not have to have input parameters for
 * both the object and data names (see Parser.C/h).
 */
class ReporterData
{
public:
  class WriteKey
  {
    /**
     * An object that can be passed to FEProblemBase::getReporterData to provide non-const access
     * the ReporterData object that is used to store reporter values.
     *
     * An attempt was made to limit access to the ReporterData as much as possible to encourage
     * developers to use the Reporter and ReporterInterface. This key also prevents the need for
     * FEProblemBase to give friend access to these classes, which gives complete access.
     * Thanks for the idea @loganharbour
     */
    WriteKey() {} // private constructor
    friend class Reporter;
    friend class ReporterInterface;
    friend class VectorPostprocessor;
    friend class VectorPostprocessorInterface;
    friend class PostprocessorInterface;
    friend class MultiAppReporterTransferBase;
  };

  ReporterData(MooseApp & moose_app);

  /**
   * Return True if a Reporter value with the given type and name have been created.
   */
  template <typename T>
  bool hasReporterValue(const ReporterName & reporter_name) const;

  /**
   * Return True if a Reporter value with any type exists with the given name.
   */
  bool hasReporterValue(const ReporterName & reporter_name) const;

  /**
   * Return a list of all reporter names
   */
  std::set<ReporterName> getReporterNames() const;

  /**
   * Method for returning read only references to Reporter values.
   * @tparam T The Reporter value C++ type.
   * @param reporter_name The name of the reporter value, which includes the object name and the
   *                      data name.
   * @param object_name The name of the object consuming the Reporter value (for error reporting)
   * @param mode The mode that the value will be consumed by the by the ReporterInterface object
   * @param time_index (optional) When not provided or zero is provided the current value is
   *                   returned. If an index greater than zero is provided then the corresponding
   *                   old data is returned (1 = old, 2 = older, etc.).
   */
  template <typename T>
  const T & getReporterValue(const ReporterName & reporter_name,
                             const std::string & object_name,
                             const ReporterMode & mode,
                             const std::size_t time_index = 0);

  /**
   * Method for returning a read-only reference to Reporter values that already exist.
   * @tparam T The Reporter value C++ type.
   * @param reporter_name The name of the reporter value, which includes the object name and the
   *                      data name.
   * @param time_index (optional) When not provided or zero is provided the current value is
   *                   returned. If an index greater than zero is provided then the corresponding
   *                   old data is returned (1 = old, 2 = older, etc.).
   */
  template <typename T>
  const T & getReporterValue(const ReporterName & reporter_name,
                             const std::size_t time_index = 0) const;

  /**
   * Method for setting Reporter values that already exist.
   * @tparam T The Reporter value C++ type.
   * @param reporter_name The name of the reporter value, which includes the object name and the
   *                      data name.
   * @param value The value to which the Reporter will be changed to.
   * @param time_index (optional) When not provided or zero is provided the current value is
   *                   returned. If an index greater than zero is provided then the corresponding
   *                   old data is returned (1 = old, 2 = older, etc.).
   * WARNING!
   * This method is designed for setting values outside of the traditional interfaces such as is
   * necessary for Transfers. This is an advanced capability that should be used with caution.
   *
   * @see FEProblemBase::setPostprocessorValueByName
   */
  template <typename T>
  void setReporterValue(const ReporterName & reporter_name,
                        const T & value,
                        const std::size_t time_index = 0);

  ///@{
  /**
   * Method for returning a writable reference to the current Reporter value. This method is
   * used by the Reporter class to produce values.
   * @tparam T The Reporter value C++ type.
   * @tparam S (optional) The ReporterContext for performing specialized actions after the values
   *           have been computed. For example, ReporterBroadcastContext automatically broadcasts
   *           the computed value. See ReporterState.C/h for more information.
   * @param reporter_name The name of the reporter value, which includes the object name and the
   *                      data name.
   * @param mode The mode that the produced value will be computed by the Reporter object
   * @param args (optional) Any number of optional arguments passed into the Context type given
   *             by the S template parameter. If S = ReporterContext then the first argument
   *             can be used as the default value (see ReporterContext.h).
   *
   * The ReporterContext objects allow for custom handling of data (e.g., broadcasting the value).
   * The get/declare methods can be called in any order thus an the underlying RestartableData
   * object is often created by the get method before it is declared. Therefore the custom
   * functionality cannot be handled by specializing the RestartableData/ReporterState object
   * directly because the state is often created prior to the declaration that dictates how the
   * produced value shall be computed. Thus, the reason for the separate ReporterContext objects.
   */
  template <typename T, template <typename> class S, typename... Args>
  T & declareReporterValue(const ReporterName & reporter_name,
                           const ReporterMode & mode,
                           Args &&... args);

  /**
   * Helper function for performing post calculation actions via the ReporterContext objects.
   *
   * If you recall, the original VectorPostprocessor system included the ability to perform some
   * scatter and broadcast actions via the special call on the storage helper object. This
   * is a replacement for that method that leverages the RepoorterContext objects to perform
   * value specific actions, including some automatic operations depending how the data is
   * produced and consumed.
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
   * Return a set of undeclared names
   *
   * @see FEProblemBase::checkPostprocessors
   */
  std::set<ReporterName> getUndeclaredNames() const;

  /**
   * Writes all Reporter values to the supplied JSON node for output to a file.
   *
   * @see JSONOutput
   */
  void store(nlohmann::json & json) const;

  /**
   * Perform integrity check for get/declare calls
   */
  void check() const;

  /**
   * Return true if the supplied mode exists in the produced Repoter values
   *
   * @see JSONOutput.C/h
   */
  bool hasReporterWithMode(const ReporterMode & mode) const;

  /**
   * Helper for performing generic Reporter value transfers.
   * @param from_name The name of the Reporter, within this instance, to transfer from
   * @param to_name The name of the reporter to transfer into
   * @param to_data The ReporterData object to transfer into   *
   * @param to_index The time index of the reporter to transfer into
   *
   * This relies on the virtual ReporterContextBase::transfer method to allow this method to
   * operate without knowledge of the type being transferred
   *
   * see MultiAppReporterTransferBase ReporterContext
   */
  void transfer(const ReporterName & from_name,
                const ReporterName & to_name,
                ReporterData & to_data,
                unsigned int to_index = 0) const;

  /**
   * Helper for performing generic Reporter value transfers.
   * @param mode The desired consumer mode to be attached to reporter
   * @param object_name The reporter name to attach the mode
   *
   * This relies on the virtual ReporterContextBase::addConsumerMode method to allow this method
   * to operate without knowledge of the type being transferred. It is needed by the Transfer to
   * mark reporter values consumer mode to ensure that the correct parallel operations will
   * occur prior to transfer.
   *
   * see MultiAppReporterTransferBase ReporterContext
   */
  void addConsumerMode(ReporterMode mode, const std::string & object_name);

private:
  /// For accessing the restart/recover system, which is where Reporter values are stored
  MooseApp & _app;

  /**
   * Helper method for creating the necessary RestartableData for Reporter values.
   * @tparam T The desired C++ type for the Reporter value
   * @param reporter_name Object/data name for the Reporter value
   * @param declare Flag indicating if the ReporterValue is being declared or read. This flag
   *                is passed to the existing MooseApp restart/recover system that errors if a
   *                value is declared multiple times.
   */
  template <typename T>
  ReporterState<T> & getReporterStateHelper(const ReporterName & reporter_name, bool declare);
  friend class VectorPostprocessorInterface;

  /**
   * Helper method for returning the ReporterContextBase object, if it exists.
   * @param reporter_name Object/data name for the Reporter value
   */
  const ReporterContextBase *
  getReporterContextBaseHelper(const ReporterName & reporter_name) const;

  /**
   * Helper for registering data with the MooseApp to avoid cyclic includes
   */
  RestartableDataValue & getRestartableDataHelper(std::unique_ptr<RestartableDataValue> data_ptr,
                                                  bool declare) const;

  /// The ReporterContext objects are created when a value is declared. The context objects
  /// include a reference to the associated ReporterState values. This container stores the
  /// context object for each Reporter value.
  ///
  /// This container must be consistently sorted across processors to achieve a consistent iteration
  /// order for parallel operations as well as for output. Objects are inserted based
  /// on the combined ReporterName. A std::list was selected to achieve fast insert times
  /// without re-allocating. A std::vector can also be used, but this was more fun. Do not use a
  /// std::set, like I did initially, since the order is actually differs in each rank because it
  /// is sorted by the pointer which causes all sorts of problems.
  std::list<std::unique_ptr<ReporterContextBase>> _context_ptrs;

  /// Names of objects that have been declared
  std::set<ReporterName> _declare_names;
  std::set<ReporterName> _get_names;
};

template <typename T>
ReporterState<T> &
ReporterData::getReporterStateHelper(const ReporterName & reporter_name, bool declare)
{
  // Creates the RestartableData object for storage in the MooseApp restart/recover system
  auto data_ptr = libmesh_make_unique<ReporterState<T>>(reporter_name);
  RestartableDataValue & value = getRestartableDataHelper(std::move(data_ptr), declare);
  auto & state_ref = static_cast<ReporterState<T> &>(value);
  return state_ref;
}

template <typename T>
const T &
ReporterData::getReporterValue(const ReporterName & reporter_name,
                               const std::string & object_name,
                               const ReporterMode & mode,
                               const std::size_t time_index)
{
  _get_names.insert(reporter_name);
  ReporterState<T> & state_ref = getReporterStateHelper<T>(reporter_name, false);
  if (mode != REPORTER_MODE_UNSET)
    state_ref.addConsumerMode(mode, object_name);
  return state_ref.value(time_index);
}

template <typename T, template <typename> class S, typename... Args>
T &
ReporterData::declareReporterValue(const ReporterName & reporter_name,
                                   const ReporterMode & mode,
                                   Args &&... args)
{
  // Update declared names list
  _declare_names.insert(reporter_name);

  // Get/create the ReporterState
  ReporterState<T> & state_ref = getReporterStateHelper<T>(reporter_name, true);

  // Create the ReporterContext
  auto context_ptr = libmesh_make_unique<S<T>>(_app, state_ref, args...);
  context_ptr->init(mode); // initialize the mode, see ContextReporter

  // Locate the insert position (see comment for _context_ptrs in ReporterData.h))
  auto func = [reporter_name](const std::unique_ptr<ReporterContextBase> & ptr) {
    return reporter_name < ptr->name();
  };
  const auto ptr = std::find_if(_context_ptrs.begin(), _context_ptrs.end(), func);
  _context_ptrs.emplace(ptr, std::move(context_ptr));
  return state_ref.value();
}

template <typename T>
bool
ReporterData::hasReporterValue(const ReporterName & reporter_name) const
{
  auto ptr = getReporterContextBaseHelper(reporter_name);
  if (ptr != nullptr)
  {
    auto context = dynamic_cast<const ReporterContext<T> *>(ptr);
    return context != nullptr;
  }
  return false;
}

template <typename T>
const T &
ReporterData::getReporterValue(const ReporterName & reporter_name,
                               const std::size_t time_index) const
{
  auto ptr = getReporterContextBaseHelper(reporter_name);
  if (ptr == nullptr)
    mooseError("The desired Reporter value '", reporter_name, "' does not exist.");
  auto context_ptr = static_cast<const ReporterContext<T> *>(ptr);
  return context_ptr->state().value(time_index);
}

template <typename T>
void
ReporterData::setReporterValue(const ReporterName & reporter_name,
                               const T & value,
                               const std::size_t time_index)
{
  // https://stackoverflow.com/questions/123758/how-do-i-remove-code-duplication-between-similar-const-and-non-const-member-func
  const auto & me = *this;
  const_cast<T &>(me.getReporterValue<T>(reporter_name, time_index)) = value;
}

// This is defined here to avoid cyclic includes, see ReporterContext.h
template <typename T>
void
ReporterContext<T>::transfer(ReporterData & r_data,
                             const ReporterName & r_name,
                             unsigned int time_index) const
{
  r_data.setReporterValue<T>(r_name, _state.value(), time_index);
}
