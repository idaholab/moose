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
#include "nlohmann/json.h"

class ReporterData;

/**
 * This is a helper class for perforing the time updates of the Reporter value as well as to aid
 * with parallel communication of the produced values. The communication aspect includes inspecting
 * the produced mode against the consumed mode to be sure that the data is compatible or if
 * automatic communication can be done to make them compatible.
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

  /// Called by InitReporterAction via ReporterData
  virtual void init() = 0;

  /// Called by FEProblemBase::advanceState via ReporterData
  virtual void copyValuesBack() = 0;

  /// Called by FEProblemBase::advanceState via ReporterData
  virtual void performAutoFinalizeOperations() = 0;

  /// Called by ReporterData::store to envoke storage of values for output
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
  virtual void finalize() = 0; // new ReporterContext objects should be override
};

/**
 * General context that is called by all Reporter values to manage the old values.
 */
template <typename T>
class ReporterContext : public ReporterContextBase
{
public:
  enum class AutoOperation
  {
    NONE,
    BROADCAST
  };
  ReporterContext(const libMesh::ParallelObject & other, ReporterState<T> & state);
  ReporterContext(const libMesh::ParallelObject & other,
                  ReporterState<T> & state,
                  const T & default_value);
  virtual const ReporterName & name() const final;
  const ReporterState<T> & state() const;
  virtual void finalize() {}

protected:
  /// The state on which this context object operates
  ReporterState<T> & _state;

private:
  /// Automatic action needed after finalize
  ReporterContext::AutoOperation _auto_operation = ReporterContext::AutoOperation::NONE;

  // The following methods are called by the ReporterData and are not intented external use, as such
  // they are changed to private. See the comments in ReporterState for why these are here rather
  // than in the ReporterState object directly.
  virtual void init() final;
  virtual void copyValuesBack() final;
  virtual void performAutoFinalizeOperations() final;
  virtual void store(nlohmann::json & json) const final;
  virtual std::string type() const final;
  friend class ReporterData;
};

template <typename T>
ReporterContext<T>::ReporterContext(const libMesh::ParallelObject & other, ReporterState<T> & state)
  : ReporterContextBase(other), _state(state)
{
}

template <typename T>
ReporterContext<T>::ReporterContext(const libMesh::ParallelObject & other,
                                    ReporterState<T> & state,
                                    const T & default_value)
  : ReporterContext(other, state)
{
  _state.get().first = default_value;
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
ReporterContext<T>::init()
{
  // Initialize all the old values
  T & value = _state.set().first;
  std::vector<T> & old_values = _state.set().second;
  old_values.resize(_state.getMaxRequestedTimeIndex());
  for (std::size_t i = 0; i < old_values.size(); ++i)
    old_values[i] = value;

  // Set the default producer mode to ROOT
  if (_state.getProducerMode() == Moose::ReporterMode::UNSET)
    _state.setProducerMode(Moose::ReporterMode::ROOT);

  // Determine auto parallel operation to perform
  const Moose::ReporterMode producer = _state.getProducerMode();
  for (const auto & pair : _state.getConsumerModes())
  {
    const Moose::ReporterMode consumer = pair.first;
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
    if (producer == Moose::ReporterMode::ROOT && consumer == Moose::ReporterMode::REPLICATED)
      _auto_operation = ReporterContext::AutoOperation::BROADCAST;

    // The following are not support and create an error
    //            ROOT -> DISTRIBUTED
    //      REPLICATED -> DISTRIBUTED
    //     DISTRIBUTED -> ROOT
    //     DISTRIBUTED -> REPLICATED
    else if ((producer == Moose::ReporterMode::ROOT &&
              consumer == Moose::ReporterMode::DISTRIBUTED) ||
             (producer == Moose::ReporterMode::REPLICATED &&
              consumer == Moose::ReporterMode::DISTRIBUTED) ||
             (producer == Moose::ReporterMode::DISTRIBUTED &&
              consumer == Moose::ReporterMode::ROOT) ||
             (producer == Moose::ReporterMode::DISTRIBUTED &&
              consumer == Moose::ReporterMode::REPLICATED))
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
}

template <typename T>
void
ReporterContext<T>::performAutoFinalizeOperations()
{
  if (_auto_operation == ReporterContext::AutoOperation::BROADCAST)
    this->comm().broadcast(this->_state.set().first);
}

template <typename T>
void
ReporterContext<T>::copyValuesBack()
{
  T & value = _state.set().first;
  std::vector<T> & old_values = _state.set().second;

  for (std::size_t i = 1; i < old_values.size(); ++i)
    old_values[i] = old_values[i - 1];

  if (old_values.size() > 0)
    old_values[0] = value;
}

template <typename T>
void
ReporterContext<T>::store(nlohmann::json & json) const
{
  storeHelper(json, this->_state.get().first);
}

template <typename T>
std::string
ReporterContext<T>::type() const
{
  return demangle(typeid(T).name());
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
  if (state.getProducerMode() != Moose::ReporterMode::UNSET)
    mooseError(
        "Reporter values using the ReporterBroadcastConxtext must not set the Reporter mode.");
  state.setProducerMode(Moose::ReporterMode::REPLICATED);
}

template <typename T>
ReporterBroadcastContext<T>::ReporterBroadcastContext(const libMesh::ParallelObject & other,
                                                      ReporterState<T> & state,
                                                      const T & default_value)
  : ReporterContext<T>(other, state, default_value)
{
  if (state.getProducerMode() != Moose::ReporterMode::UNSET)
    mooseError(
        "Reporter values using the ReporterBroadcastConxtext must not set the Reporter mode.");
  state.setProducerMode(Moose::ReporterMode::REPLICATED);
}

template <typename T>
void
ReporterBroadcastContext<T>::finalize()
{
  ReporterContext<T>::finalize();
  this->comm().broadcast(this->_state.set().first);
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
  if (state.getProducerMode() != Moose::ReporterMode::UNSET)
    mooseError("Reporter values using the ReporterScatterContext must not set the Reporter mode.");
  state.setProducerMode(Moose::ReporterMode::DISTRIBUTED);
}

template <typename T>
ReporterScatterContext<T>::ReporterScatterContext(const libMesh::ParallelObject & other,
                                                  ReporterState<T> & state,
                                                  const T & default_value,
                                                  const std::vector<T> & values)
  : ReporterContext<T>(other, state, default_value), _values(values)
{
  if (state.getProducerMode() != Moose::ReporterMode::UNSET)
    mooseError("Reporter values using the ReporterScatterContext must not set the Reporter mode.");
  state.setProducerMode(Moose::ReporterMode::DISTRIBUTED);
}

template <typename T>
void
ReporterScatterContext<T>::finalize()
{
  mooseAssert(this->processor_id() == 0 ? _values.size() == this->n_processors() : true,
              "Vector to be scatter must be sized to match the number of processors");
  mooseAssert(this->processor_id() > 0 ? _values.size() == 0 : true,
              "Vector to be scatter must be sized to on processors execpt for the root processor");
  ReporterContext<T>::finalize();
  this->comm().scatter(_values, this->_state.set().first);
}

/**
 * A context that gathers the Reporter value to the root processor
 */
template <typename T>
class ReporterGatherContext : public ReporterContext<T>
{
public:
  ReporterGatherContext(const libMesh::ParallelObject & other,
                        ReporterState<T> & state,
                        const T & value);
  ReporterGatherContext(const libMesh::ParallelObject & other,
                        ReporterState<T> & state,
                        const T & default_value,
                        const T & value);

  virtual void finalize() override;

private:
  /// The values to Gather
  const T & _value;
};

template <typename T>
ReporterGatherContext<T>::ReporterGatherContext(const libMesh::ParallelObject & other,
                                                ReporterState<T> & state,
                                                const T & value)
  : ReporterContext<T>(other, state), _value(value)
{
  if (state.getProducerMode() != Moose::ReporterMode::UNSET)
    mooseError("Reporter values using the ReporterGatherContext must not set the Reporter mode.");
  state.setProducerMode(Moose::ReporterMode::ROOT);
}

template <typename T>
ReporterGatherContext<T>::ReporterGatherContext(const libMesh::ParallelObject & other,
                                                ReporterState<T> & state,
                                                const T & default_value,
                                                const T & value)
  : ReporterContext<T>(other, state, default_value), _value(value)
{
  if (state.getProducerMode() != Moose::ReporterMode::UNSET)
    mooseError("Reporter values using the ReporterGatherContext must not set the Reporter mode.");
  state.setProducerMode(Moose::ReporterMode::ROOT);
}

template <typename T>
void
ReporterGatherContext<T>::finalize()
{
  ReporterContext<T>::finalize();
  this->_state.set().first = _value;
  this->comm().gather(0, this->_state.set().first);
}
