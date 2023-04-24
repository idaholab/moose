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
#include "JsonIO.h"
#include "MooseUtils.h"
#include "ReporterState.h"
#include "ReporterContext.h"
#include "libmesh/parallel_object.h"
#include "libmesh/dense_vector.h"
#include <memory>

class MooseApp;
class Receiver;

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
 * time history. A "context" object also exists that uses the ReporterState for performing special
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
    friend class Postprocessor;
    friend class Receiver;
    friend class VectorPostprocessor;
    friend class ReporterTransferInterface;
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
   * @returns True if a ReporterState is defined for the Reporter with name \p reporter_name
   * and the given type.
   */
  template <typename T>
  bool hasReporterState(const ReporterName & reporter_name) const;

  /**
   * @returns True if a ReporterState is defined for the Reporter with name \p reporter_name.
   */
  bool hasReporterState(const ReporterName & reporter_name) const;

  /**
   * Return a list of all reporter names
   */
  std::set<ReporterName> getReporterNames() const;

  /**
   * Return a list of all postprocessor names
   */
  std::set<std::string> getPostprocessorNames() const;

  /**
   * Get all real reporter values including postprocessor and vector postprocessor values into a
   * dense vector
   */
  DenseVector<Real> getAllRealReporterValues() const;

  /**
   * Get full names of all real reporter values
   * Note: For a postprocessor, the full name is the postprocessor name plus '/value'.
   *       For a vector postprocessor, the full name is the vector postprocessor name
   *       plus the vector name followed by '/#' where '#' is the index of the vector.
   */
  std::vector<std::string> getAllRealReporterFullNames() const;

  /**
   * Method for returning read only references to Reporter values.
   * @tparam T The Reporter value C++ type.
   * @param reporter_name The name of the reporter value, which includes the object name and the
   *                      data name.
   * @param consumer The MooseObject consuming the Reporter value (for error reporting)
   * @param mode The mode that the value will be consumed by the by the ReporterInterface object
   * @param time_index (optional) When not provided or zero is provided the current value is
   *                   returned. If an index greater than zero is provided then the corresponding
   *                   old data is returned (1 = old, 2 = older, etc.).
   */
  template <typename T>
  const T & getReporterValue(const ReporterName & reporter_name,
                             const MooseObject & consumer,
                             const ReporterMode & mode,
                             const std::size_t time_index = 0) const;

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

  /**
   * Method for setting that a specific time index is requested for a Reporter value.
   * @tparam T The Reporter value C++ type.
   * @param reporter_name The name of the reporter value, which includes the object name and the
   *                      data name.
   * @param time_index The time index that is needed
   */
  template <typename T>
  void needReporterTimeIndex(const ReporterName & reporter_name, const std::size_t time_index);

  ///@{
  /**
   * Method for returning a writable reference to the current Reporter value. This method is
   * used by the Reporter class to produce values.
   * @tparam T The Reporter value C++ type.
   * @tparam S (optional) The ReporterContext for performing specialized actions after the
   * values have been computed. For example, ReporterBroadcastContext automatically broadcasts
   *           the computed value. See ReporterState.C/h for more information.
   * @param reporter_name The name of the reporter value, which includes the object name and the
   *                      data name.
   * @param mode The mode that the produced value will be computed by the Reporter object
   * @param producer The MooseObject that produces this value
   * @param args (optional) Any number of optional arguments passed into the Context type given
   *             by the S template parameter. If S = ReporterContext then the first argument
   *             can be used as the default value (see ReporterContext.h).
   *
   * The ReporterContext objects allow for custom handling of data (e.g., broadcasting the
   * value). The get/declare methods can be called in any order thus an the underlying
   * RestartableData object is often created by the get method before it is declared. Therefore
   * the custom functionality cannot be handled by specializing the
   * RestartableData/ReporterState object directly because the state is often created prior to
   * the declaration that dictates how the produced value shall be computed. Thus, the reason
   * for the separate ReporterContext objects.
   */
  template <typename T, typename S, typename... Args>
  T & declareReporterValue(const ReporterName & reporter_name,
                           const ReporterMode & mode,
                           const MooseObject & producer,
                           Args &&... args);

  /**
   * Helper function for performing post calculation actions via the ReporterContext objects.
   *
   * If you recall, the original VectorPostprocessor system included the ability to perform some
   * scatter and broadcast actions via the special call on the storage helper object. This
   * is a replacement for that method that leverages the ReporterContext objects to perform
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
   * Perform integrity check for get/declare calls
   */
  void check() const;

  /**
   * Return true if the supplied mode exists in the produced Reporter values
   *
   * @see CSV.C/h
   */
  bool hasReporterWithMode(const std::string & obj_name, const ReporterMode & mode) const;

  /**
   * @returns The ReporterContextBase associated with the Reporter with name \p reporter_name.
   */
  ///@{
  const ReporterContextBase & getReporterContextBase(const ReporterName & reporter_name) const;
  ReporterContextBase & getReporterContextBase(const ReporterName & reporter_name);
  ///@}

  /**
   * @Returns The ReporterStateBase associated with the Reporter with name \p reporter_name.
   */
  ///@{
  const ReporterStateBase & getReporterStateBase(const ReporterName & reporter_name) const;
  ReporterStateBase & getReporterStateBase(const ReporterName & reporter_name);
  ///@}

  /**
   * Return the ReporterProducerEnum for an existing ReporterValue
   * @param reporter_name The name of the reporter value, which includes the object name and the
   *                      data name.
   */
  const ReporterProducerEnum & getReporterMode(const ReporterName & reporter_name) const;

  /**
   * Gets information pertaining to the Reporter with state \p state and possibly
   * context \p context.
   */
  static std::string getReporterInfo(const ReporterStateBase & state,
                                     const ReporterContextBase * context);

  /**
   * Gets information pertaining to the Reporter with name \p reporter_name.
   */
  std::string getReporterInfo(const ReporterName & reporter_name) const;

  /**
   * Gets information about all declared/requested Reporters.
   */
  std::string getReporterInfo() const;

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
   * @param moose_object The object requesting/declaring the state, used in error handling.
   */
  template <typename T>
  ReporterState<T> & getReporterStateHelper(const ReporterName & reporter_name,
                                            bool declare,
                                            const MooseObject * moose_object = nullptr) const;

  /**
   * Helper for registering data with the MooseApp to avoid cyclic includes
   */
  RestartableDataValue & getRestartableDataHelper(std::unique_ptr<RestartableDataValue> data_ptr,
                                                  bool declare) const;

  /// Map from ReporterName -> Reporter state. We need to keep track of all of the states that are
  /// created so that we can check them after Reporter declaration to make sure all states have
  /// a producer (are delcared). We cannot check _context_ptrs, because a context is only
  /// defined for Reporters that have been declared. This is mutable so that it can be inserted
  /// into when requesting Reporter values.
  mutable std::map<ReporterName, ReporterStateBase *> _states;

  /// The ReporterContext objects are created when a value is declared. The context objects
  /// include a reference to the associated ReporterState values. This container stores the
  /// context object for each Reporter value.
  std::map<ReporterName, std::unique_ptr<ReporterContextBase>> _context_ptrs;
};

template <typename T>
ReporterState<T> &
ReporterData::getReporterStateHelper(const ReporterName & reporter_name,
                                     bool declare,
                                     const MooseObject * moose_object /* = nullptr */) const
{
  if (hasReporterState(reporter_name))
  {
    const auto error_helper =
        [this, &reporter_name, &moose_object, &declare](const std::string & suffix)
    {
      std::stringstream oss;
      oss << "While " << (declare ? "declaring" : "requesting") << " a "
          << reporter_name.specialTypeToName() << " value with the name \""
          << reporter_name.getValueName() << "\"";
      if (!reporter_name.isPostprocessor() && !reporter_name.isVectorPostprocessor())
        oss << " and type \"" << MooseUtils::prettyCppType<T>() << "\"";
      oss << ",\na Reporter with the same name " << suffix << ".\n\n";
      oss << getReporterInfo(reporter_name);

      if (moose_object)
        moose_object->mooseError(oss.str());
      else
        mooseError(oss.str());
    };

    if (declare && hasReporterValue(reporter_name))
      error_helper("has already been declared");
    if (!hasReporterState<T>(reporter_name))
    {
      std::stringstream oss;
      oss << "has been " << (declare || !hasReporterValue(reporter_name) ? "requested" : "declared")
          << " with a different type";
      error_helper(oss.str());
    }
  }

  // Reporter states are stored as restartable data. The act of registering restartable data
  // may be done multiple times with the same name, which will happen when more than one
  // get value is done, or a get value and a declare is done. With this, we create a new
  // state every time, but said created state may not be the actual state if this state
  // is already registered as restartable data. Therefore, we create a state, and then
  // cast the restartable data received back to a state (which may be different than
  // the one we created, but that's okay)
  auto state_unique_ptr = std::make_unique<ReporterState<T>>(reporter_name);
  auto & restartable_value = getRestartableDataHelper(std::move(state_unique_ptr), declare);

  auto * state = dynamic_cast<ReporterState<T> *>(&restartable_value);
  mooseAssert(state, "Cast failed. The check above must be broken!");

  // See declareReporterValue for a comment on what happens if a state for the same
  // name is requested but with different special types. TLDR: ReporterNames with
  // different special types are not unique so they'll be the same entry
  _states.emplace(reporter_name, state);

  return *state;
}

template <typename T>
const T &
ReporterData::getReporterValue(const ReporterName & reporter_name,
                               const MooseObject & consumer,
                               const ReporterMode & mode,
                               const std::size_t time_index /* = 0 */) const
{
  auto & state = getReporterStateHelper<T>(reporter_name, /* declare = */ false, &consumer);
  state.addConsumer(mode, consumer);
  return state.value(time_index);
}

template <typename T, typename S, typename... Args>
T &
ReporterData::declareReporterValue(const ReporterName & reporter_name,
                                   const ReporterMode & mode,
                                   const MooseObject & producer,
                                   Args &&... args)
{
  // Get/create the ReporterState
  auto & state = getReporterStateHelper<T>(reporter_name, /* declare = */ true, &producer);

  // They key in _states (ReporterName) is not unique by special type. This is done on purpose
  // because we want to store reporter names a single name regardless of special type.
  // Because of this, we have the case where someone could request a reporter value
  // that is later declared as a pp or a vpp value. In this case, when it is first
  // requested, the _state entry will have a key and name with a special type of ANY.
  // When it's declared here (later), we will still find the correct entry because
  // we don't check the special type in the key/name. But... we want the actual
  // key and name to represent a pp or a vpp. Therefore, we'll set it properly,
  // remove the entry in the map (which has the ANY key), and re-add it so that it
  // has the pp/vpp key. This allows us to identify Reporters that really represent
  // pps/vpps in output and in error reporting.
  if (reporter_name.isPostprocessor() && !state.getReporterName().isPostprocessor())
  {
    state.setIsPostprocessor();
    _states.erase(reporter_name);
    _states.emplace(reporter_name, &state);
  }
  else if (reporter_name.isVectorPostprocessor() &&
           !state.getReporterName().isVectorPostprocessor())
  {
    state.setIsVectorPostprocessor();
    _states.erase(reporter_name);
    _states.emplace(reporter_name, &state);
  }

  mooseAssert(!_context_ptrs.count(reporter_name), "Context already exists");

  // Create the ReporterContext
  auto context_ptr = std::make_unique<S>(_app, producer, state, args...);
  context_ptr->init(mode); // initialize the mode, see ContextReporter
  _context_ptrs.emplace(reporter_name, std::move(context_ptr));

  return state.value();
}

template <typename T>
bool
ReporterData::hasReporterValue(const ReporterName & reporter_name) const
{
  if (!hasReporterValue(reporter_name))
    return false;
  return dynamic_cast<const ReporterContext<T> *>(&getReporterContextBase(reporter_name));
}

template <typename T>
bool
ReporterData::hasReporterState(const ReporterName & reporter_name) const
{
  if (!hasReporterState(reporter_name))
    return false;
  return dynamic_cast<const ReporterState<T> *>(&getReporterStateBase(reporter_name));
}

template <typename T>
const T &
ReporterData::getReporterValue(const ReporterName & reporter_name,
                               const std::size_t time_index) const
{
  if (!hasReporterValue<T>(reporter_name))
    mooseError("Reporter name \"",
               reporter_name,
               "\" with type \"",
               MooseUtils::prettyCppType<T>(),
               "\" is not declared.");

  // Force the const version of value, which does not allow for increasing time index
  return static_cast<const ReporterState<T> &>(
             getReporterStateHelper<T>(reporter_name, /* declare = */ false))
      .value(time_index);
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

template <typename T>
void
ReporterData::needReporterTimeIndex(const ReporterName & reporter_name,
                                    const std::size_t time_index)
{
  getReporterValue<T>(reporter_name, 0); // for error checking that it is declared
  getReporterStateHelper<T>(reporter_name, /* declare = */ false).value(time_index);
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

// This is defined here to avoid cyclic includes, see ReporterContext.h
template <typename T>
void
ReporterContext<T>::transferToVector(ReporterData & r_data,
                                     const ReporterName & r_name,
                                     dof_id_type index,
                                     unsigned int time_index) const
{
  std::vector<T> & vec =
      const_cast<std::vector<T> &>(r_data.getReporterValue<std::vector<T>>(r_name, time_index));

  if (index >= vec.size())
    mooseError(
        "Requested index ", index, " is outside the bounds of the vector reporter value ", r_name);
  vec[index] = _state.value();
}

// This is defined here to avoid cyclic includes, see ReporterContext.h
template <typename T>
void
ReporterGeneralContext<T>::declareClone(ReporterData & r_data,
                                        const ReporterName & r_name,
                                        const ReporterMode & mode,
                                        const MooseObject & producer) const
{
  r_data.declareReporterValue<T, ReporterGeneralContext<T>>(r_name, mode, producer);
}

// This is defined here to avoid cyclic includes, see ReporterContext.h
template <typename T>
void
ReporterGeneralContext<T>::declareVectorClone(ReporterData & r_data,
                                              const ReporterName & r_name,
                                              const ReporterMode & mode,
                                              const MooseObject & producer) const
{
  r_data.declareReporterValue<std::vector<T>, ReporterVectorContext<T>>(r_name, mode, producer);
}

// This is defined here to avoid cyclic includes, see ReporterContext.h
template <typename T>
void
ReporterVectorContext<T>::declareClone(ReporterData &,
                                       const ReporterName &,
                                       const ReporterMode &,
                                       const MooseObject &) const
{
  mooseError("Cannot create clone with ReporterVectorContext.");
}

// This is defined here to avoid cyclic includes, see ReporterContext.h
template <typename T>
void
ReporterVectorContext<T>::declareVectorClone(ReporterData &,
                                             const ReporterName &,
                                             const ReporterMode &,
                                             const MooseObject &) const
{
  mooseError("Cannot create clone with ReporterVectorContext.");
}
