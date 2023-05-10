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
#include "JsonIO.h"
#include "JsonSyntaxTree.h"
#include "MooseObject.h"

class ReporterData;

/**
 * This is a helper class to aid with parallel communication of compute Reporter values as well
 * as provides a link to the stored Reporter value state object.
 *
 * @see Reporter, ReporterData, ReporterState
 *
 * This file contains several reporter context types with specific polymorphism:
 *
 * ReporterContextBase--->ReporterContext<T>
 *                        |        |
 *                        |        ReporterGeneralContext<T>
 *                        |        |        |      |
 *                        |        |        |      ReporterGatherContext<T>
 *                        |        |        ReporterScatterContext<T>
 *                        |        ReporterBroadcastContext<T>
 *                        |
 *                        ReporterVectorContext<std::vector<T>>
 *
 * When creating a new context, it is generally advisable to derive from ReporterGeneralContext
 * (@see VectorPostprocessorContext). The reason for the split between ReporterGeneralContext
 * and ReporterVectorContext is due to the declareVectorClone and resize functionality. If we
 * were to declare a vector clone in ReporterContext there would be a infinite instantiation of
 * of vector contexts, which is why this declare is defined in ReporterGeneralContext and an
 * error is thrown in ReporterVectorContext. There is also no easy way to partially instantiate
 * a member function (you have to due it for the entire class), which is why the resize is
 * defined in ReporterVectorContext. That being said, we are always open for improvements,
 * especially for simplifying the polymorphism of these contexts.
 */
class ReporterContextBase : public libMesh::ParallelObject
{
public:
  ReporterContextBase(const libMesh::ParallelObject & other, const MooseObject & producer);
  virtual ~ReporterContextBase() = default;

  /// Return the ReporterName that the context is associated
  virtual const ReporterName & name() const = 0;

  /// Return the type of the data stored
  // This is a helper for ReporterContext::storeInfo
  virtual std::string type() const = 0;

  /**
   * @returns The derived context type for this context.
   *
   * This must be overridden in every class.
   */
  virtual std::string contextType() const = 0;

  /// Called by FEProblemBase::advanceState via ReporterData
  virtual void copyValuesBack() = 0;

  /// Called by JSONOutput::outputReporters to output meta data independent of calculated
  /// values
  virtual void storeInfo(nlohmann::json & json) const = 0;

  /// Called by JSONOutput::outputReporters to invoke storage of values for output
  ///
  /// This method exists and is distinct from the RestartableData::store method for JSON output
  /// via the JSONOutput object. The RestartableData::store/load methods are designed for restart
  /// and include all the data including the old values.
  ///
  /// This method only outputs the current value within the JSONOutput object.
  /// NOTE: nlohmann qualification is needed for argument json because the std::vector overload is
  ///       not in the std namespace, it's in the nlohmann namespace, and will come up in argument
  ///       dependent lookup (ADL) only because of this qualification.
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
   * Return the MooseObject that produces this Reporter.
   */
  const MooseObject & getProducer() const { return _producer; }

  /**
   * Return the Reporter value produced mode
   */
  const ReporterProducerEnum & getProducerModeEnum() const;

  /**
   * Helper for enabling generic transfer of Reporter values
   * @param r_data The ReporterData on the app that this data is being transferred to
   * @param r_name The name of the Report being transferred to
   *
   * @see MultiAppReporterTransfer
   */
  virtual void transfer(ReporterData & r_data,
                        const ReporterName & r_name,
                        unsigned int time_index = 0) const = 0;

  /**
   * Helper for enabling generic transfer of Reporter values to a vector
   * @param r_data The ReporterData on the app that this data is being transferred to
   * @param r_name The name of the Report being transfered to
   *
   * @see ReporterTransferInterface
   */
  virtual void transferToVector(ReporterData & r_data,
                                const ReporterName & r_name,
                                dof_id_type index,
                                unsigned int time_index = 0) const = 0;

  /**
   * Helper for declaring new reporter values based on this context
   *
   * @param r_data The ReporterData on the app that this value is being declared
   * @param r_name The name of the Reporter value being declared
   * @param mode Reporter mode to declare with
   *
   * @see ReporterTransferInterface
   */
  virtual void declareClone(ReporterData & r_data,
                            const ReporterName & r_name,
                            const ReporterMode & mode,
                            const MooseObject & producer) const = 0;

  /**
   * Helper for declaring new vector reporter values based on this context
   *
   * @param r_data The ReporterData on the app that this value is being declared
   * @param r_name The name of the Reporter value being declared
   * @param mode Reporter mode to declare with
   *
   * @see ReporterTransferInterface
   */
  virtual void declareVectorClone(ReporterData & r_data,
                                  const ReporterName & r_name,
                                  const ReporterMode & mode,
                                  const MooseObject & producer) const = 0;

  /**
   * Helper for resizing vector data
   *
   * @param local_size Number of elements to resize vector to
   */
  virtual void resize(dof_id_type local_size) = 0;

protected:
  /**
   * Helper for checking whether or not the state \p state has only the modes \p modes.
   */
  void requiresConsumerModes(const ReporterStateBase & state,
                             const std::set<ReporterMode> & modes) const;

  /// The MooseObject that is producing this Reporter
  const MooseObject & _producer;

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

  ReporterContext(const libMesh::ParallelObject & other,
                  const MooseObject & producer,
                  ReporterState<T> & state);
  ReporterContext(const libMesh::ParallelObject & other,
                  const MooseObject & producer,
                  ReporterState<T> & state,
                  const T & default_value);

  /**
   * Return the name of the Reporter value
   */
  const ReporterName & name() const override final { return _state.getReporterName(); }

  /**
   * Return a reference to the ReporterState object that is storing the Reporter value
   */
  const ReporterState<T> & state() const { return _state; }

  /**
   * Return the type being stored by the associated ReporterState object.
   *
   * @see ReporterContext::storeInfo
   */
  virtual std::string type() const override { return MooseUtils::prettyCppType<T>(); }

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
   * Perform type specific transfer to a vector
   *
   * NOTE: This is defined in ReporterData.h to avoid cyclic includes that would arise. I don't
   *       know of a better solution, if you have one please implement it.
   */
  virtual void transferToVector(ReporterData & r_data,
                                const ReporterName & r_name,
                                dof_id_type index,
                                unsigned int time_index = 0) const override;

protected:
  void broadcast()
  {
    if constexpr (MooseUtils::canBroadcast<T>::value)
      this->comm().broadcast(this->_state.value());
    else
      mooseError("Cannot broadcast Reporter type '", MooseUtils::prettyCppType<T>(), "'");
  }

  /// Output meta data to JSON, see JSONOutput
  virtual void storeInfo(nlohmann::json & json) const override;

  /// Output data to JSON, see JSONOutput
  virtual void store(nlohmann::json & json) const override;

  virtual std::string contextType() const override = 0;

  // The following are called by the ReporterData and are not indented for external use
  virtual void copyValuesBack() override;

  /// The state on which this context object operates
  ReporterState<T> & _state;
};

template <typename T>
ReporterContext<T>::ReporterContext(const libMesh::ParallelObject & other,
                                    const MooseObject & producer,
                                    ReporterState<T> & state)
  : ReporterContextBase(other, producer), _state(state)
{
  _producer_enum.insert(REPORTER_MODE_ROOT, REPORTER_MODE_REPLICATED, REPORTER_MODE_DISTRIBUTED);
}

template <typename T>
ReporterContext<T>::ReporterContext(const libMesh::ParallelObject & other,
                                    const MooseObject & producer,
                                    ReporterState<T> & state,
                                    const T & default_value)
  : ReporterContext(other, producer, state)
{
  _state.value() = default_value;
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
  for (const auto & pair : _state.getConsumers())
  {
    const ReporterMode consumer = pair.first;
    const MooseObject * moose_object = pair.second;

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
      mooseError("The Reporter value \"",
                 name(),
                 "\" is being produced in ",
                 producer,
                 " mode, but the ",
                 moose_object->typeAndName(),
                 " is requesting to consume it in ",
                 consumer,
                 " mode, which is not supported.");
  }

  // Perform desired auto parallel operation
  if (auto_operation == ReporterContext::AutoOperation::BROADCAST)
    this->broadcast();
}

template <typename T>
void
ReporterContext<T>::copyValuesBack()
{
  _state.copyValuesBack();
}

template <typename T>
void
ReporterContext<T>::storeInfo(nlohmann::json & json) const
{
  json["type"] = this->type();
}

template <typename T>
void
ReporterContext<T>::store(nlohmann::json & json) const
{
  nlohmann::to_json(json, this->_state.value());
}

template <typename T>
class ReporterGeneralContext : public ReporterContext<T>
{
public:
  ReporterGeneralContext(const libMesh::ParallelObject & other,
                         const MooseObject & producer,
                         ReporterState<T> & state)
    : ReporterContext<T>(other, producer, state)
  {
  }
  ReporterGeneralContext(const libMesh::ParallelObject & other,
                         const MooseObject & producer,
                         ReporterState<T> & state,
                         const T & default_value)
    : ReporterContext<T>(other, producer, state, default_value)
  {
  }

  /**
   * Declare a reporter value of same type as this context.
   *
   * NOTE: This is defined in ReporterData.h to avoid cyclic includes that would arise. I don't
   *       know of a better solution, if you have one please implement it.
   */
  virtual void declareClone(ReporterData & r_data,
                            const ReporterName & r_name,
                            const ReporterMode & mode,
                            const MooseObject & producer) const override;
  /**
   * Declare a reporter value that is a vector of the same type as this context.
   *
   * NOTE: This is defined in ReporterData.h to avoid cyclic includes that would arise. I don't
   *       know of a better solution, if you have one please implement it.
   */
  virtual void declareVectorClone(ReporterData & r_data,
                                  const ReporterName & r_name,
                                  const ReporterMode & mode,
                                  const MooseObject & producer) const override;

  virtual void resize(dof_id_type local_size) final;

  virtual std::string contextType() const override { return MooseUtils::prettyCppType(this); }
};

template <typename T>
void ReporterGeneralContext<T>::resize(dof_id_type)
{
  mooseError("Cannot resize non vector-type reporter values.");
}

/**
 * A context that broadcasts the Reporter value from the root processor
 */
template <typename T>
class ReporterBroadcastContext : public ReporterGeneralContext<T>
{
public:
  ReporterBroadcastContext(const libMesh::ParallelObject & other,
                           const MooseObject & producer,
                           ReporterState<T> & state);
  ReporterBroadcastContext(const libMesh::ParallelObject & other,
                           const MooseObject & producer,
                           ReporterState<T> & state,
                           const T & default_value);
  virtual void finalize() override;
  virtual std::string contextType() const override { return MooseUtils::prettyCppType(this); }
};

template <typename T>
ReporterBroadcastContext<T>::ReporterBroadcastContext(const libMesh::ParallelObject & other,
                                                      const MooseObject & producer,
                                                      ReporterState<T> & state)
  : ReporterGeneralContext<T>(other, producer, state)
{
  this->_producer_enum.clear();
  this->_producer_enum.insert(REPORTER_MODE_ROOT);
}

template <typename T>
ReporterBroadcastContext<T>::ReporterBroadcastContext(const libMesh::ParallelObject & other,
                                                      const MooseObject & producer,
                                                      ReporterState<T> & state,
                                                      const T & default_value)
  : ReporterGeneralContext<T>(other, producer, state, default_value)
{
  this->_producer_enum.clear();
  this->_producer_enum.insert(REPORTER_MODE_ROOT);
}

template <typename T>
void
ReporterBroadcastContext<T>::finalize()
{
  this->requiresConsumerModes(this->_state, {REPORTER_MODE_UNSET, REPORTER_MODE_REPLICATED});
  this->broadcast();
}

/**
 * A context that scatters the Reporter value from the root processor
 */
template <typename T>
class ReporterScatterContext : public ReporterGeneralContext<T>
{
public:
  ReporterScatterContext(const libMesh::ParallelObject & other,
                         const MooseObject & producer,
                         ReporterState<T> & state,
                         const std::vector<T> & values);
  ReporterScatterContext(const libMesh::ParallelObject & other,
                         const MooseObject & producer,
                         ReporterState<T> & state,
                         const T & default_value,
                         const std::vector<T> & values);

  virtual void finalize() override;
  virtual std::string contextType() const override { return MooseUtils::prettyCppType(this); }

private:
  /// The values to scatter
  const std::vector<T> & _values;
};

template <typename T>
ReporterScatterContext<T>::ReporterScatterContext(const libMesh::ParallelObject & other,
                                                  const MooseObject & producer,
                                                  ReporterState<T> & state,
                                                  const std::vector<T> & values)
  : ReporterGeneralContext<T>(other, producer, state), _values(values)
{
  this->_producer_enum.clear();
  this->_producer_enum.insert(REPORTER_MODE_ROOT);
}

template <typename T>
ReporterScatterContext<T>::ReporterScatterContext(const libMesh::ParallelObject & other,
                                                  const MooseObject & producer,
                                                  ReporterState<T> & state,
                                                  const T & default_value,
                                                  const std::vector<T> & values)
  : ReporterGeneralContext<T>(other, producer, state, default_value), _values(values)
{
  this->_producer_enum.clear();
  this->_producer_enum.insert(REPORTER_MODE_ROOT);
}

template <typename T>
void
ReporterScatterContext<T>::finalize()
{
  this->requiresConsumerModes(this->_state, {REPORTER_MODE_UNSET, REPORTER_MODE_REPLICATED});

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
class ReporterGatherContext : public ReporterGeneralContext<T>
{
public:
  ReporterGatherContext(const libMesh::ParallelObject & other,
                        const MooseObject & producer,
                        ReporterState<T> & state);
  ReporterGatherContext(const libMesh::ParallelObject & other,
                        const MooseObject & producer,
                        ReporterState<T> & state,
                        const T & default_value);

  virtual void finalize() override;
  virtual std::string contextType() const override { return MooseUtils::prettyCppType(this); }
};

template <typename T>
ReporterGatherContext<T>::ReporterGatherContext(const libMesh::ParallelObject & other,
                                                const MooseObject & producer,
                                                ReporterState<T> & state)
  : ReporterGeneralContext<T>(other, producer, state)
{
  this->_producer_enum.clear();
  this->_producer_enum.insert(REPORTER_MODE_DISTRIBUTED);
}

template <typename T>
ReporterGatherContext<T>::ReporterGatherContext(const libMesh::ParallelObject & other,
                                                const MooseObject & producer,
                                                ReporterState<T> & state,
                                                const T & default_value)
  : ReporterGeneralContext<T>(other, producer, state, default_value)
{
  this->_producer_enum.clear();
  this->_producer_enum.insert(REPORTER_MODE_DISTRIBUTED);
}

template <typename T>
void
ReporterGatherContext<T>::finalize()
{
  this->requiresConsumerModes(this->_state, {REPORTER_MODE_UNSET, REPORTER_MODE_ROOT});
  this->comm().gather(0, this->_state.value());
}

/**
 * This context is specific for vector types of reporters, mainly for declaring a vector
 * of the type from another context. As well as resizing the vector of data.
 *
 * @see ReporterGeneralContext::declareVectorClone and ReporterTransferInterface
 */
template <typename T>
class ReporterVectorContext : public ReporterContext<std::vector<T>>
{
public:
  ReporterVectorContext(const libMesh::ParallelObject & other,
                        const MooseObject & producer,
                        ReporterState<std::vector<T>> & state);
  ReporterVectorContext(const libMesh::ParallelObject & other,
                        const MooseObject & producer,
                        ReporterState<std::vector<T>> & state,
                        const std::vector<T> & default_value);

  /**
   * This simply throws an error to avoid infinite instantiations.
   * It is defined in ReporterData.h to avoid cyclic included.
   */
  virtual void declareClone(ReporterData & r_data,
                            const ReporterName & r_name,
                            const ReporterMode & mode,
                            const MooseObject & producer) const final;

  /**
   * This simply throws an error to avoid infinite instantiations.
   * It is defined in ReporterData.h to avoid cyclic included.
   */
  virtual void declareVectorClone(ReporterData & r_data,
                                  const ReporterName & r_name,
                                  const ReporterMode & mode,
                                  const MooseObject & producer) const final;

  /**
   * Since we know that the _state value is a vector type, we can resize it based
   * on @param local_size
   */
  virtual void resize(dof_id_type local_size) override { this->_state.value().resize(local_size); }

  virtual std::string contextType() const override { return MooseUtils::prettyCppType(this); }
};

template <typename T>
ReporterVectorContext<T>::ReporterVectorContext(const libMesh::ParallelObject & other,
                                                const MooseObject & producer,
                                                ReporterState<std::vector<T>> & state)
  : ReporterContext<std::vector<T>>(other, producer, state)
{
}

template <typename T>
ReporterVectorContext<T>::ReporterVectorContext(const libMesh::ParallelObject & other,
                                                const MooseObject & producer,
                                                ReporterState<std::vector<T>> & state,
                                                const std::vector<T> & default_value)
  : ReporterContext<std::vector<T>>(other, producer, state, default_value)
{
}
