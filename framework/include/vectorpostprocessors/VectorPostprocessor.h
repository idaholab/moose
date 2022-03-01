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
#include "OutputInterface.h"
#include "MooseEnum.h"
#include "ReporterContext.h"

// libMesh
#include "libmesh/parallel.h"

// Forward declarations
class FEProblemBase;
class InputParameters;
class SamplerBase;
class VectorPostprocessorData;

template <typename T>
InputParameters validParams();

/**
 * Base class for Postprocessors that produce a vector of values.
 */
class VectorPostprocessor : public OutputInterface
{
public:
  static InputParameters validParams();

  VectorPostprocessor(const MooseObject * moose_object);

  virtual ~VectorPostprocessor() = default;

  /**
   * Returns the name of the VectorPostprocessor.
   */
  std::string PPName() const { return _vpp_name; }

  /**
   * Return whether or not this VectorPostprocessor contains complete history
   */
  bool containsCompleteHistory() const { return _contains_complete_history; }

  /**
   * Return true if the VPP is operating in distributed mode.
   */
  bool isDistributed() const { return _is_distributed; }

  /**
   * Return the names of the vectors associated with this object.
   */
  const std::set<std::string> & getVectorNames() const;

protected:
  /**
   * Register a new vector to fill up.
   */
  VectorPostprocessorValue & declareVector(const std::string & vector_name);

  /// The name of the VectorPostprocessor
  const std::string _vpp_name;

  /// The FEProblemBase
  FEProblemBase & _vpp_fe_problem;

  /// DISTRIBUTED or REPLICATED
  const MooseEnum & _parallel_type;

  friend class SamplerBase;

private:
  const MooseObject & _vpp_moose_object;

  const THREAD_ID _vpp_tid;

  const bool _contains_complete_history;

  const bool _is_distributed;

  const bool _is_broadcast;

  std::map<std::string, VectorPostprocessorValue> _thread_local_vectors;

  std::set<std::string> _vector_names;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
// The following itmes were created to maintain the various getScatter methods in the
// VectorPostprocessorInterface.

// Special consumer modue add
extern const ReporterMode REPORTER_MODE_VPP_SCATTER;

/*
 * Special ReporterContext that performs a scatter operation if a getScatter method is called on the
 * VPP value.
 *
 * The source file contains an explicit instantiation.
 * @see VectorPostprocessorInterface
 */
template <typename T>
class VectorPostprocessorContext : public ReporterGeneralContext<T>
{
public:
  VectorPostprocessorContext(const libMesh::ParallelObject & other,
                             const MooseObject & producer,
                             ReporterState<T> & state);
  virtual void finalize() override;
  virtual void copyValuesBack() override;

  const ScatterVectorPostprocessorValue & getScatterValue() const;
  const ScatterVectorPostprocessorValue & getScatterValueOld() const;

private:
  ScatterVectorPostprocessorValue _scatter_value;
  ScatterVectorPostprocessorValue _scatter_value_old;
};
