//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "OutputInterface.h"
#include "ReporterData.h"
#include "InputParameters.h"

// System includes
#include <type_traits>

// Forward declarations
class FEProblemBase;

/**
 * Reporter objects allow for the declaration of arbitrary data types that are aggregate values
 * for a simulation. These aggregate values are then available to other objects for use. They
 * operate with the typical producer/consumer relationship. The Reporter object produces values
 * that other objects consume.
 *
 * Originally, MOOSE included a Postprocessor system that allowed for an object to produce a
 * single scalar value for consumption by other objects. Then a companion system was created,
 * the VectorPostprocessor system, that allowed for an object to produce many std::vector<Real>
 * values. The Reporter system is the generalization of these two ideas and follows closely the
 * original design of the VectorPostprocessor system.
 *
 * In practice, the Reporter system provided the following features over the two previous systems.
 *
 * 1. It can create arbitrary data types rather than only Real and std::vector<Real>
 * 2. It can create many values per object. This was possible for VectorPostprocessor objects but
 *    not for Postprocessors. Therefore, this allows a single Reporter to provide multiple values
 *    and consolidate similar items. For example, a "counter" object replaced several individual
 *    Postprocessor objects.
 * 3. The ReporterContext system allows for each data value to have special operation within a
 *    single object. Previously the VectorPostprocessor system had capability to perform
 *    broadcasting, but it was applied to all vectors in the object.
 */
class Reporter : public OutputInterface
{
public:
  static InputParameters validParams();
  Reporter(const MooseObject * moose_object);
  virtual ~Reporter() = default;
  virtual void store(nlohmann::json & json) const;

  /**
   * @returns Whether or not this Reporter should store its value at this specific time.
   *
   * Basic Reporters (those that are not GeneralReporters) will store at all times
   * when requested.
   */
  virtual bool shouldStore() const { return true; }

protected:
  ///@{
  /**
   * Method to define a value that the Reporter object is to produced.
   * @tparam T The C++ type of the value to be produced
   * @tparam S (optional) Context type for performing special operations. For example
   *           using the ReporterBroadcastContext will automatically broadcast the produced value
   *           from the root processor.
   * @param name A unique name for the value to be produced.
   * @param mode (optional) The mode that indicates how the value produced is represented in
   *             parallel, there is more information about this below
   * @param args (optional) Any number of optional arguments passed into the ReporterContext given
   *             by the S template parameter. If S = ReporterContext then the first argument
   *             can be used as the default value (see ReporterContext.h).
   *
   * The 'mode' indicates how the value that is produced is represented in parallel. It is the
   * responsibility of the Reporter object to get it to that state. The ReporterContext objects
   * are designed to help with this. The mode can be one of the following:
   *
   *     ReporterMode::ROOT Indicates that the value produced is complete/correct on the
   *                        root processor for the object.
   *     ReporterMode::REPLICATED Indicates that the value produced is complete/correct on
   *                              all processors AND that the value is the same on all
   *                              processors
   *     ReporterMode::DISTRIBUTED Indicates that the value produced is complete/correct on
   *                               all processors AND that the value is NOT the same on all
   *                               processors
   *
   * WARNING! Using the "default value" in ReporterContext:
   * The Reporter system, like the systems before it, allow for objects that consume values to be
   * constructed prior to the produce objects. When a value is requested either by a producer
   * (Reporter) or consumer (ReporterInterface) the data is allocated. As such the assigned default
   * value from the producer should not be relied upon on the consumer side during object
   * construction.
   *
   * NOTE:
   * The ReporterContext is just a mechanism to allow for handling of values in special ways. In
   * practice it might be better to have specific methods for these special cases. For example,
   * a declareBroadcastValue, etc. Please refer to the ReporterData object for more information
   * on how the data system operates for Reporter values.
   */
  template <typename T, template <typename> class S = ReporterGeneralContext, typename... Args>
  T & declareValue(const std::string & param_name, Args &&... args);
  template <typename T, template <typename> class S = ReporterGeneralContext, typename... Args>
  T & declareValue(const std::string & param_name, ReporterMode mode, Args &&... args);
  template <typename T, template <typename> class S = ReporterGeneralContext, typename... Args>
  T & declareValueByName(const ReporterValueName & value_name, Args &&... args);
  template <typename T, template <typename> class S = ReporterGeneralContext, typename... Args>
  T & declareValueByName(const ReporterValueName & value_name, ReporterMode mode, Args &&... args);

  template <typename T, typename S, typename... Args>
  T & declareValue(const std::string & param_name, Args &&... args);
  template <typename T, typename S, typename... Args>
  T & declareValue(const std::string & param_name, ReporterMode mode, Args &&... args);
  template <typename T, typename S, typename... Args>
  T & declareValueByName(const ReporterValueName & value_name, Args &&... args);
  template <typename T, typename S, typename... Args>
  T & declareValueByName(const ReporterValueName & value_name, ReporterMode mode, Args &&... args);
  ///@}

  /**
   * Declare a unused value with type T.
   *
   * This is useful when you have a reporter that has optional values. In this case,
   * you want to create references to all reporter values. However, because some values
   * are optional, you need _something_ to fill into the reference. This helper will
   * create a unused value. It also allows for the passing of arguments in the case
   * that your value is not trivially default constructable (constructable by default
   * without arguments).
   */
  template <typename T, typename... Args>
  T & declareUnusedValue(Args &&... args);

private:
  /**
   * Internal base struct for use in storing unused values.
   *
   * In order to store a vector of arbitrary unused values for declareUnusedValue(),
   * we need some base object that is constructable without template arguments.
   */
  struct UnusedWrapperBase
  {
    /// Needed for polymorphism
    virtual ~UnusedWrapperBase() {}
  };

  /**
   * Internal struct for storing a unused value. This allows for the storage
   * of arbitrarily typed objects in a single vector for use in
   * declareUnusedValue().
   */
  template <typename T>
  struct UnusedWrapper : UnusedWrapperBase
  {
    T value;
  };

  /**
   * @returns The ReporterValueName associated with the parameter \p param_name.
   *
   * Performs error checking on if the parameter is valid.
   */
  const ReporterValueName & getReporterValueName(const std::string & param_name) const;

  /// The MooseObject creating this Reporter
  const MooseObject & _reporter_moose_object;

  /// Ref. to MooseObject params
  const InputParameters & _reporter_params;

  /// The name of the MooseObject, from "_object_name" param
  const std::string & _reporter_name;

  /// Needed for access to FEProblemBase::getReporterData
  FEProblemBase & _reporter_fe_problem;

  /// Data storage
  ReporterData & _reporter_data;

  /// Storage for unused values declared with declareUnusedValue().
  std::vector<std::unique_ptr<UnusedWrapperBase>> _unused_values;
};

template <typename T, template <typename> class S, typename... Args>
T &
Reporter::declareValue(const std::string & param_name, Args &&... args)
{
  return declareValue<T, S<T>>(param_name, REPORTER_MODE_UNSET, args...);
}

template <typename T, template <typename> class S, typename... Args>
T &
Reporter::declareValue(const std::string & param_name, ReporterMode mode, Args &&... args)
{
  return declareValue<T, S<T>>(param_name, mode, args...);
}

template <typename T, typename S, typename... Args>
T &
Reporter::declareValue(const std::string & param_name, Args &&... args)
{
  return declareValue<T, S>(param_name, REPORTER_MODE_UNSET, args...);
}

template <typename T, typename S, typename... Args>
T &
Reporter::declareValue(const std::string & param_name, ReporterMode mode, Args &&... args)
{
  return declareValueByName<T, S>(getReporterValueName(param_name), mode, args...);
}

template <typename T, template <typename> class S, typename... Args>
T &
Reporter::declareValueByName(const ReporterValueName & value_name, Args &&... args)
{
  return declareValueByName<T, S<T>>(value_name, REPORTER_MODE_UNSET, args...);
}

template <typename T, template <typename> class S, typename... Args>
T &
Reporter::declareValueByName(const ReporterValueName & value_name,
                             ReporterMode mode,
                             Args &&... args)
{
  return declareValueByName<T, S<T>>(value_name, mode, args...);
}

template <typename T, typename S, typename... Args>
T &
Reporter::declareValueByName(const ReporterValueName & value_name, Args &&... args)
{
  return declareValueByName<T, S>(value_name, REPORTER_MODE_UNSET, args...);
}

template <typename T, typename S, typename... Args>
T &
Reporter::declareValueByName(const ReporterValueName & value_name,
                             ReporterMode mode,
                             Args &&... args)
{
  const ReporterName state_name(_reporter_name, value_name);

  buildOutputHideVariableList({state_name.getCombinedName()});

  return _reporter_data.declareReporterValue<T, S>(
      state_name, mode, _reporter_moose_object, args...);
}

template <typename T, typename... Args>
T &
Reporter::declareUnusedValue(Args &&... args)
{
  _unused_values.emplace_back(std::make_unique<UnusedWrapper<T>>(std::forward(args)...));
  UnusedWrapper<T> * wrapper = dynamic_cast<UnusedWrapper<T> *>(_unused_values.back().get());
  return wrapper->value;
}
