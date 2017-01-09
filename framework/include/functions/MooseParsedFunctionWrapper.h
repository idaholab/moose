/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MOOSEPARSEDFUNCTIONWRAPPER_H
#define MOOSEPARSEDFUNCTIONWRAPPER_H

// std includes
#include <string>
#include <vector>

// MOOSE includes
#include "ParallelUniqueId.h"
#include "MooseError.h"

// libMesh includes
#include "libmesh/parsed_function.h"

// Forward declarations
class FEProblemBase;

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
  template<typename T>
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

  /// Storage for the values
  std::vector<Real> _vals;

  /// Pointer to the libMesh::ParsedFunction object
  std::unique_ptr<ParsedFunction<Real>> _function_ptr;

  /// Stores the relative location of variables (in _vars) that are connected to Postprocessors
  std::vector<unsigned int> _pp_index;

  /// Vector of pointers to PP values
  std::vector<const Real *> _pp_vals;

  /// Stores the relative location of variables (in _vars) that are connected to Scalar Variables
  std::vector<unsigned int> _scalar_index;

  /// Vector of pointers to PP values
  std::vector<Real *> _scalar_vals;

  /// Vector of pointers to the variables in libMesh::ParsedFunction
  std::vector<Real *> _addr;

  /// The thread id passed from owning Function object
  const THREAD_ID _tid;

  /**
   * Initialization method that prepares the vars and vals for use
   * by the libMesh::ParsedFunction object allocated in the constructor
   */
  void initialize();

  /**
   * Updates postprocessor values for use in the libMesh::ParsedFunction
   */
  void update();

  // moose_unit needs access
  friend class ParsedFunctionTest;
};

/**
 * The general evaluation method is not defined.
 */
template<typename T>
T evaluate(Real /*t*/, const Point & /*p*/)
{
  mooseError("The evaluate method is not defined for this type.");
}

#endif // MOOOSEPARSEDFUNCTIONWRAPPER_H
