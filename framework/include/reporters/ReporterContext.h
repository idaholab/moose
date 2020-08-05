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

#include "libmesh/parallel.h"
#include "libmesh/parallel_object.h"

#include "ReporterName.h"

/**
 * The ReporterData helper class uses the a context object to perform special operations such as
 * shrinking the old/older value storage in the general case or performing broadcasting of values.
 */
class ReporterContextBase : public libMesh::ParallelObject
{
public:
  ReporterContextBase(const libMesh::ParallelObject & other);
  virtual ~ReporterContextBase() = default;

  /// Return the ReporterName that the context is associated
  virtual const ReporterName & name() const = 0;

  /// Called by InitReporterAction via ReporterData
  virtual void init() = 0;

  /// Called by FEProblemBase::advanceState via ReporterData
  virtual void copyValuesBack() = 0;

  /// Called by FEProblem::joinAndFinalize via ReporterData
  virtual void finalize() = 0;
};

/**
 * General context that is called by all Reporter values to manage the old values.
 *
 * If creating new context classes be sure to call the base class methods so that handling
 * of the old values is not lost.
 */
template <typename T>
class ReporterContext : public ReporterContextBase
{
public:
  ReporterContext(const libMesh::ParallelObject & other, ReporterState<T> & state);
  ReporterContext(const libMesh::ParallelObject & other, ReporterState<T> & state, const T & default_value);
  virtual const ReporterName & name() const final;
  virtual void init() override;
  virtual void copyValuesBack() override;
  virtual void finalize() override {}

protected:
  /// The state on which this context object operates
  ReporterState<T> & _state;
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
void
ReporterContext<T>::init()
{
  T & value = _state.set().first;
  std::vector<T> & old_values = _state.set().second;

  old_values.resize(_state.getMaxRequestedTimeIndex());

  for (std::size_t i = 0; i < old_values.size(); ++i)
    old_values[i] = value;
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

/**
 * A context that broadcasts the Reporter value from the root processor
 */
template <typename T>
class ReporterBroadcastContext : public ReporterContext<T>
{
public:
  ReporterBroadcastContext(const libMesh::ParallelObject & other, ReporterState<T> & state);
  ReporterBroadcastContext(const libMesh::ParallelObject & other, ReporterState<T> & state, const T & default_value);
  virtual void finalize() override;
};

template <typename T>
ReporterBroadcastContext<T>::ReporterBroadcastContext(const libMesh::ParallelObject & other,
                                                      ReporterState<T> & state)
  : ReporterContext<T>(other, state)
{
}

template <typename T>
ReporterBroadcastContext<T>::ReporterBroadcastContext(const libMesh::ParallelObject & other,
                                                      ReporterState<T> & state,
                                                      const T & default_value)
  : ReporterContext<T>(other, state, default_value)
{
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
}

template <typename T>
ReporterScatterContext<T>::ReporterScatterContext(const libMesh::ParallelObject & other,
                                                  ReporterState<T> & state,
                                                  const T & default_value,
                                                  const std::vector<T> & values)
  : ReporterContext<T>(other, state, default_value), _values(values)
{
}

template <typename T>
void
ReporterScatterContext<T>::finalize()
{
  mooseAssert(this->processor_id() == 0 ? _values.size() == this->n_processors() : true, "Vector to be scatter must be sized to match the number of processors");
  mooseAssert(this->processor_id() > 0 ? _values.size() == 0 : true, "Vector to be scatter must be sized to on processors execpt for the root processor");
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
}

template <typename T>
ReporterGatherContext<T>::ReporterGatherContext(const libMesh::ParallelObject & other,
                                                ReporterState<T> & state,
                                                const T & default_value,
                                                const T & value)
  : ReporterContext<T>(other, state, default_value), _value(value)
{
}

template <typename T>
void
ReporterGatherContext<T>::finalize()
{
  ReporterContext<T>::finalize();
  this->_state.set().first = _value;
  this->comm().gather(0, this->_state.set().first);
}
