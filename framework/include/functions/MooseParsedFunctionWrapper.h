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
#include "MooseError.h"
#include "MooseTypes.h"

#include "libmesh/parsed_function.h"

// C++ includes
#include <string>
#include <vector>

// Forward declarations
class FEProblemBase;
class Function;

/**
 * A wrapper class for creating and evaluating parsed functions via the
 * libMesh::ParsedFunction interface for fparser.
 * @see MooseParsedFunction
 * @see MooseParsedGradFunction
 * @see MooseParsedVectorFunction
 */
class MooseParsedFunctionWrapper
{
public:
  /**
   * Class constructor
   * @param feproblem Reference to the FEProblemBase object (provides access to Postprocessors)
   * @param function_str A string that contains the function to evaluate
   * @param vars A vector of variable names contained within the function
   * @param vals A vector of variable values, matching the variables defined in vars
   */
  MooseParsedFunctionWrapper(FEProblemBase & feproblem,
                             const std::string & function_str,
                             const std::vector<std::string> & vars,
                             const std::vector<std::string> & vals,
                             const THREAD_ID tid = 0);

  /**
   * Class destruction
   * Deletes the pointer to the dynamically allocated instance of the
   * underlying libMesh::ParsedFunction
   */
  virtual ~MooseParsedFunctionWrapper();

  /**
   * A template method for performing the evaluation of the libMesh::ParsedFunction
   * Within the source two specializations exists for returning a scalar or vector; template
   * specialization was utilized to allow for generic expansion.
   */
  template <typename T>
  T evaluate(Real t, const Point & p);

  /**
   * Evaluate the gradient of the function which libMesh provides through
   * automatic differentiation
   */
  RealGradient evaluateGradient(Real t, const Point & p);

  /**
   * Evaluate the time derivative of the function which libMesh provides through
   * automatic differentiation
   */
  Real evaluateDot(Real t, const Point & p);

private:
  /// Reference to the FEProblemBase object
  FEProblemBase & _feproblem;

  /// Reference to the string containing the function to evaluate
  const std::string & _function_str;

  /// List of variables supplied from the user
  const std::vector<std::string> & _vars;

  /// List of the values for the variables supplied by the user
  const std::vector<std::string> & _vals_input;

  /// Storage for the initial values of _vars variables used by the libMesh::ParsedFunction object
  std::vector<Real> _initial_vals;

  /// Pointer to the libMesh::ParsedFunction object
  std::unique_ptr<libMesh::ParsedFunction<Real>> _function_ptr;

  /// Stores indices into _addr variable that are connected to Postprocessors
  std::vector<unsigned int> _pp_index;

  /// Vector of pointers to postprocessor values this parsed function is using
  std::vector<const Real *> _pp_vals;

  /// Stores indicies into _addr variable that are connected to Scalar Variables
  std::vector<unsigned int> _scalar_index;

  /// Vector of pointers to scalar variables values
  std::vector<const Real *> _scalar_vals;

  /// Stores indices into _addr that are connected to Functions this libMesh::ParsedFunction is using
  std::vector<unsigned int> _function_index;

  /// Vector of Functions this parsed function is using
  std::vector<const Function *> _functions;

  /// Pointers to the variables that store the values of _vars inside the libMesh::ParsedFunction object
  std::vector<Real *> _addr;

  /// The thread id passed from owning Function object
  const THREAD_ID _tid;

  /**
   * Initialization method that prepares the _vars and _initial_vals for use
   * by the libMesh::ParsedFunction object allocated in the constructor
   */
  void initialize();

  /**
   * Updates postprocessor and scalar values for use in the libMesh::ParsedFunction
   */
  void update();

  /**
   * Updates function values for use in the libMesh::ParsedFunction
   */
  void updateFunctionValues(Real t, const Point & pt);

  // moose_unit needs access
  friend class ParsedFunctionTest;
};

/**
 * The general evaluation method is not defined.
 */
template <typename T>
T
evaluate(Real /*t*/, const Point & /*p*/)
{
  mooseError("The evaluate method is not defined for this type.");
}

template <>
Real MooseParsedFunctionWrapper::evaluate(Real t, const Point & p);

template <>
DenseVector<Real> MooseParsedFunctionWrapper::evaluate(Real t, const Point & p);

template <>
RealVectorValue MooseParsedFunctionWrapper::evaluate(Real t, const Point & p);
