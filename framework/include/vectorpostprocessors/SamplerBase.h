//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseTypes.h"

// Forward Declarations
class InputParameters;
class VectorPostprocessor;
class TransientBase;

namespace libMesh
{
class Point;

namespace Parallel
{
class Communicator;
}
}

template <typename T>
InputParameters validParams();

/**
 * Base class for VectorPostprocessors that need to do "sampling" of
 * values in the domain.
 */
class SamplerBase
{
public:
  static InputParameters validParams();

  /**
   * @param parameters The parameters for the object
   * @param vpp A pointer to the child object
   * @param comm The communicator of the child
   */
  SamplerBase(const InputParameters & parameters,
              VectorPostprocessor * vpp,
              const libMesh::Parallel::Communicator & comm);
  virtual ~SamplerBase() = default;

protected:
  /**
   * You MUST call this in the constructor of the child class and pass down the name
   * of the variables.
   *
   * @param variable_names The names of the variables.  Note: The order of the variables sets the
   * order of the values for addSample()
   */
  void setupVariables(const std::vector<std::string> & variable_names);

  /**
   *  Checks whether the passed variable pointer corresponds to a regular single-valued field
   * variable
   * @param var_param_name name of the variable parameter in which the variables were passed
   * @param var_ptr pointer to the field variable
   */
  void checkForStandardFieldVariableType(const MooseVariableFieldBase * const var_ptr,
                                         const std::string & var_param_name = "variable") const;

  /**
   * Call this with the value of every variable at each point you want to sample at.
   * @param p The point where you took the sample
   * @param id This can either be an actual ID or a distance or anything else you want
   * @param values The value of each variable
   */
  virtual void addSample(const Point & p, const Real & id, const std::vector<Real> & values);

  /**
   * Initialize the datastructures.
   *
   * YOU MUST CALL THIS DURING initialize() in the child class!
   */
  virtual void initialize();

  /**
   * Finalize the values.
   *
   * YOU MUST CALL THIS DURING finalize() in the child class!
   */
  virtual void finalize();

  /**
   * Join the values.
   *
   * YOU MUST CALL THIS DURING threadJoin() in the child class!
   *
   * @param y You must cast the UserObject to your child class type first then you can pass it in
   * here.
   */
  virtual void threadJoin(const SamplerBase & y);

  /// The child params
  const InputParameters & _sampler_params;

  /// The child VectorPostprocessor
  VectorPostprocessor * _vpp;

  /// The communicator of the child
  const libMesh::Parallel::Communicator & _comm;

  /// Transient executioner used to determine if the last solve converged
  const TransientBase * const _sampler_transient;

  /// The variable names
  std::vector<std::string> _variable_names;

  /// What to sort by
  const unsigned int _sort_by;

  /// x coordinate of the points
  VectorPostprocessorValue & _x;
  /// y coordinate of the points
  VectorPostprocessorValue & _y;
  /// x coordinate of the points
  VectorPostprocessorValue & _z;

  /// The node ID of each point
  VectorPostprocessorValue & _id;

  std::vector<VectorPostprocessorValue *> _values;

private:
  /// The number of samples added in the last execution
  std::size_t _curr_num_samples = 0;
  /// The indices of the samples in the last execution
  std::set<std::size_t, std::greater<std::size_t>> _curr_indices;
  /// The full size of the vector since the last execution
  std::size_t _curr_total_samples = 0;
};
