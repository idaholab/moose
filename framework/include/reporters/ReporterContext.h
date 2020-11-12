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
#include <typeinfo>

#include "libmesh/parallel.h"
#include "libmesh/parallel_object.h"

#include "ReporterName.h"
#include "ReporterMode.h"
#include "ReporterState.h"
#include "nlohmann/json.h"
#include "JsonSyntaxTree.h"

class ReporterData;

/**
 * This is a helper class to aid with parallel communication of compute Reporter values as well
 * as provides a link to the stored Reporter value state object.
 *
 * @see Reporter, ReporterData, ReporterState
 */
class ReporterContextBase : public libMesh::ParallelObject
{
public:
  ReporterContextBase(const libMesh::ParallelObject & other);
  virtual ~ReporterContextBase() = default;

  /// Return the ReporterName that the context is associated
  virtual const ReporterName & name() const = 0;

  /// Return the type of the data stored
  // This is a helper for ReporterData::store
  virtual std::string type() const = 0;

  /// Called by FEProblemBase::advanceState via ReporterData
  virtual void copyValuesBack() = 0;

  /// Called by ReporterData::store to invoke storage of values for output
  ///
  /// This method exists and is distinct from the RestartableData::store method for JSON output
  /// via the JSONOutput object. The RestartableData::store/load methods are designed for restart
  /// and include all the data including the old values.
  ///
  /// This method only outputs the current value along with other information within the JSONOutput
  /// object.
  ///
  /// @see JsonIO.h
  /// @see JSONOutput.h
  virtual void store(nlohmann::json & json) const = 0;

  /// Called by FEProblemBase::joinAndFinalize via ReporterData
  virtual void finalize() = 0; // new ReporterContext objects should override

  /**
   * Initialize the producer mode.
   *
   * This done after construction to allow the constructor to define the available values in the
   * ReporterProducerEnum.
   *
   * @see ReporterData::declareReporterValue
   */
  void init(const ReporterMode & mode);

  /**
   * Return the Reporter value produced mode
   */
  const ReporterProducerEnum & getProducerModeEnum() const;

  /**
   * Helper for enabling generic transfer of Reporter values
   * @param r_data The ReporterData on the app that this data is being transferred to
   * @param r_name The name of the Report being transfered to
   *
   * @see MultiAppReporterTransfer
   */
  virtual void transfer(ReporterData & r_data,
                        const ReporterName & r_name,
                        unsigned int time_index = 0) const = 0;

  /**
   * Helper for enabling generic transfer of Reporter values
   *
   * This allows the Transfer object to set the ReporterValue consumer mode, which ensures that
   * the data is in the proper state prior to transfer. Doing this via the override allows it to
   * occur without the transfer having knowledge of the types being transfered.
   *
   * @see MultiAppReporterTransfer
   */
  virtual void addConsumerMode(ReporterMode mode, const std::string & object_name) = 0;

protected:
  /// Defines how the Reporter value can be produced and how it is being produced
  ReporterProducerEnum _producer_enum;
};

/**
 * General context that is called by all Reporter values to manage the old values.
 *
 * This is the default context object, the communication aspect includes inspecting the produced
 * mode against the consumed mode to be sure that the data is compatible or if automatic
 * communication can be done to make them compatible.
 */
template <typename T>
class ReporterContext : public ReporterContextBase
{
public:
  /**
   * Options for automatic parallel operations to perform by the default context
   */
  enum class AutoOperation
  {
    NONE,
    BROADCAST
  };

  ReporterContext(const libMesh::ParallelObject & other, ReporterState<T> & state);
  ReporterContext(const libMesh::ParallelObject & other,
                  ReporterState<T> & state,
                  const T & default_value);

  /**
   * Return the name of the Reporter value
   */
  virtual const ReporterName & name() const final;

  /**
   * Return a reference to the ReporterState object that is storing the Reporter value
   */
  const ReporterState<T> & state() const;

  /**
   * Return the type being stored by the associated ReporterState object.
   *
   * @see ReporterData::store
   */
  virtual std::string type() const final;

  /**
   * Perform automatic parallel communication based on the producer/consumer modes
   */
  virtual void finalize() override;

  /**
   * Perform type specific transfer
   *
   * NOTE: This is defined in ReporterData.h to avoid cyclic includes that would arise. I don't
   *       know of a better solution, if you have one please implement it.
   */
  virtual void transfer(ReporterData & r_data,
                        const ReporterName & r_name,
                        unsigned int time_index = 0) const override;

  /**
   * Add a consumer mode to the associate ReporterState
   */
  virtual void addConsumerMode(ReporterMode mode, const std::string & object_name) override;

protected:
  /// The state on which this context object operates
  ReporterState<T> & _state;

  // The following are called by the ReporterData and are not indented for external use
  virtual void copyValuesBack() override;
  virtual void store(nlohmann::json & json) const override;
  friend class ReporterData;
};

template <typename T>
ReporterContext<T>::ReporterContext(const libMesh::ParallelObject & other, ReporterState<T> & state)
  : ReporterContextBase(other), _state(state)
{
  _producer_enum.insert(REPORTER_MODE_ROOT, REPORTER_MODE_REPLICATED, REPORTER_MODE_DISTRIBUTED);
}

template <typename T>
ReporterContext<T>::ReporterContext(const libMesh::ParallelObject & other,
                                    ReporterState<T> & state,
                                    const T & default_value)
  : ReporterContext(other, state)
{
  _state.value() = default_value;
}

template <typename T>
const ReporterName &
ReporterContext<T>::name() const
{
  return _state.getReporterName();
}

template <typename T>
const ReporterState<T> &
ReporterContext<T>::state() const
{
  return _state;
}

template <typename T>
void
ReporterContext<T>::finalize()
{
  // Automatic parallel operation to perform
  ReporterContext::AutoOperation auto_operation = ReporterContext::AutoOperation::NONE;

  // Set the default producer mode to ROOT
  if (!_producer_enum.isValid())
    _producer_enum.assign(REPORTER_MODE_ROOT);

  // Determine auto parallel operation to perform
  const auto & producer = _producer_enum; // convenience
  for (const auto & pair : _state.getConsumerModes())
  {
    const ReporterMode consumer = pair.first;
    const std::string & object_name = pair.second;

    // The following sets up the automatic operations and performs error checking for the various
    // modes for the producer and consumer
    //
    // The data is correct and requires no operation for the following conditions (PRODUCER ->
    // CONSUMER)
    //            ROOT -> ROOT
    //      REPLICATED -> REPLICATED
    //     DISTRIBUTED -> DISTRIBUTED
    //      REPLICATED -> ROOT

    // Perform broadcast in the case
    //            ROOT -> REPLICATED
    if (producer == REPORTER_MODE_ROOT && consumer == REPORTER_MODE_REPLICATED)
      auto_operation = ReporterContext::AutoOperation::BROADCAST;

    // The following are not support and create an error
    //            ROOT -> DISTRIBUTED
    //      REPLICATED -> DISTRIBUTED
    //     DISTRIBUTED -> ROOT
    //     DISTRIBUTED -> REPLICATED
    else if ((producer == REPORTER_MODE_ROOT && consumer == REPORTER_MODE_DISTRIBUTED) ||
             (producer == REPORTER_MODE_REPLICATED && consumer == REPORTER_MODE_DISTRIBUTED) ||
             (producer == REPORTER_MODE_DISTRIBUTED && consumer == REPORTER_MODE_ROOT) ||
             (producer == REPORTER_MODE_DISTRIBUTED && consumer == REPORTER_MODE_REPLICATED))
      mooseError("The Reporter value '",
                 name(),
                 "' is being produced in ",
                 producer,
                 " mode, but the '",
                 object_name,
                 "' object is requesting to consume it in ",
                 consumer,
                 " mode, which is not supported.");
  }

  // Perform desired auto parallel operation
  if (auto_operation == ReporterContext::AutoOperation::BROADCAST)
    this->comm().broadcast(this->_state.value());
}

template <typename T>
void
ReporterContext<T>::copyValuesBack()
{
  _state.copyValuesBack();
}

template <typename T>
void
ReporterContext<T>::store(nlohmann::json & json) const
{
  storeHelper(json, this->_state.value());
}

template <typename T>
std::string
ReporterContext<T>::type() const
{
  return JsonSyntaxTree::prettyCppType(demangle(typeid(T).name()));
}

template <typename T>
void
ReporterContext<T>::addConsumerMode(ReporterMode mode, const std::string & object_name)
{
  _state.addConsumerMode(mode, object_name);
}

/**
 * A context that broadcasts the Reporter value from the root processor
 */
template <typename T>
class ReporterBroadcastContext : public ReporterContext<T>
{
public:
  ReporterBroadcastContext(const libMesh::ParallelObject & other, ReporterState<T> & state);
  ReporterBroadcastContext(const libMesh::ParallelObject & other,
                           ReporterState<T> & state,
                           const T & default_value);
  virtual void finalize() override;
};

template <typename T>
ReporterBroadcastContext<T>::ReporterBroadcastContext(const libMesh::ParallelObject & other,
                                                      ReporterState<T> & state)
  : ReporterContext<T>(other, state)
{
  this->_producer_enum.clear();
  this->_producer_enum.insert(REPORTER_MODE_ROOT);
}

template <typename T>
ReporterBroadcastContext<T>::ReporterBroadcastContext(const libMesh::ParallelObject & other,
                                                      ReporterState<T> & state,
                                                      const T & default_value)
  : ReporterContext<T>(other, state, default_value)
{
  this->_producer_enum.clear();
  this->_producer_enum.insert(REPORTER_MODE_ROOT);
}

template <typename T>
void
ReporterBroadcastContext<T>::finalize()
{
  for (const auto & pair : this->_state.getConsumerModes())
  {
    const ReporterMode consumer = pair.first;
    const std::string & object_name = pair.second;
    if (!(consumer == REPORTER_MODE_UNSET || consumer == REPORTER_MODE_REPLICATED))
      mooseError("The Reporter value '",
                 this->name(),
                 "' is being produced in ",
                 REPORTER_MODE_ROOT,
                 " mode, but the '",
                 object_name,
                 "' object is requesting to consume it in ",
                 consumer,
                 " mode, which is not supported. The mode must be UNSET or REPLICATED.");
  }

  this->comm().broadcast(this->_state.value());
}

/**
 * A context that scatters the Reporter value from the root processor
 */
template <typename T>
class ReporterScatterContext : public ReporterContext<T>
{
public:
  ReporterScatterContext(const libMesh::ParallelObject & other,
                         ReporterState<T> & state,
                         const std::vector<T> & values);
  ReporterScatterContext(const libMesh::ParallelObject & other,
                         ReporterState<T> & state,
                         const T & default_value,
                         const std::vector<T> & values);

  virtual void finalize() override;

private:
  /// The values to scatter
  const std::vector<T> & _values;
};

template <typename T>
ReporterScatterContext<T>::ReporterScatterContext(const libMesh::ParallelObject & other,
                                                  ReporterState<T> & state,
                                                  const std::vector<T> & values)
  : ReporterContext<T>(other, state), _values(values)
{
  this->_producer_enum.clear();
  this->_producer_enum.insert(REPORTER_MODE_ROOT);
}

template <typename T>
ReporterScatterContext<T>::ReporterScatterContext(const libMesh::ParallelObject & other,
                                                  ReporterState<T> & state,
                                                  const T & default_value,
                                                  const std::vector<T> & values)
  : ReporterContext<T>(other, state, default_value), _values(values)
{
  this->_producer_enum.clear();
  this->_producer_enum.insert(REPORTER_MODE_ROOT);
}

template <typename T>
void
ReporterScatterContext<T>::finalize()
{
  for (const auto & pair : this->_state.getConsumerModes())
  {
    const ReporterMode consumer = pair.first;
    const std::string & object_name = pair.second;
    if (!(consumer == REPORTER_MODE_UNSET || consumer == REPORTER_MODE_DISTRIBUTED))
      mooseError("The Reporter value '",
                 this->name(),
                 "' is being produced in ",
                 REPORTER_MODE_ROOT,
                 " mode, but the '",
                 object_name,
                 "' object is requesting to consume it in ",
                 consumer,
                 " mode, which is not supported. The mode must be UNSET or DISTRIBUTED.");
  }

  mooseAssert(this->processor_id() == 0 ? _values.size() == this->n_processors() : true,
              "Vector to be scattered must be sized to match the number of processors");
  mooseAssert(
      this->processor_id() > 0 ? _values.size() == 0 : true,
      "Vector to be scattered must be sized to zero on processors except for the root processor");
  this->comm().scatter(_values, this->_state.value());
}

/**
 * A context that gathers the Reporter value to the root processor
 */
template <typename T>
class ReporterGatherContext : public ReporterContext<T>
{
public:
  ReporterGatherContext(const libMesh::ParallelObject & other, ReporterState<T> & state);
  ReporterGatherContext(const libMesh::ParallelObject & other,
                        ReporterState<T> & state,
                        const T & default_value);

  virtual void finalize() override;
};

template <typename T>
ReporterGatherContext<T>::ReporterGatherContext(const libMesh::ParallelObject & other,
                                                ReporterState<T> & state)
  : ReporterContext<T>(other, state)
{
  this->_producer_enum.clear();
  this->_producer_enum.insert(REPORTER_MODE_DISTRIBUTED);
}

template <typename T>
ReporterGatherContext<T>::ReporterGatherContext(const libMesh::ParallelObject & other,
                                                ReporterState<T> & state,
                                                const T & default_value)
  : ReporterContext<T>(other, state, default_value)
{
  this->_producer_enum.clear();
  this->_producer_enum.insert(REPORTER_MODE_DISTRIBUTED);
}

template <typename T>
void
ReporterGatherContext<T>::finalize()
{
  for (const auto & pair : this->_state.getConsumerModes())
  {
    const ReporterMode consumer = pair.first;
    if (!(consumer == REPORTER_MODE_UNSET || consumer == REPORTER_MODE_ROOT))
      mooseError("The Reporter value '",
                 this->name(),
                 "' is being produced in ",
                 REPORTER_MODE_DISTRIBUTED,
                 " mode, but the '",
                 pair.second, // object name
                 "' object is requesting to consume it in ",
                 consumer,
                 " mode, which is not supported. The mode must be UNSET or ROOT.");
  }
  this->comm().gather(0, this->_state.value());
}
