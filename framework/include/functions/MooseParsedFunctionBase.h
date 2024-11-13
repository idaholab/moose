//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Standard library
#include <vector>
#include <memory>

// MOOSE includes
#include "MooseError.h"

// Forward declarations
class FEProblemBase;
class InputParameters;
class MooseParsedFunctionWrapper;

/**
 * Creates the 'vars' and 'vals' parameters used by all ParsedFunctions, the
 * parameters provided from this function should be appended to the parameters
 * for the class using the += operator.
 * @see MooseParsedFunction,  MooseParsedGradFunction, MooseParsedVectorFunction
 */
template <typename T>
InputParameters validParams();

/**
 * Adds user facing parameters for parsed function
 * @see ParsedFunction ParsedVectorFunction ParsedGradFunction
 */
class MooseParsedFunctionBase
{
public:
  /**
   * Class constructor for the interface.  The first parameter, 'name' is not currently used.
   * @param parameters Input parameters from the object, it must contain '_fe_problem'
   */
  static InputParameters validParams();

  MooseParsedFunctionBase(const InputParameters & parameters);

  /**
   * Class destructor.
   */
  virtual ~MooseParsedFunctionBase();

protected:
  /**
   * A helper method to check if the function value contains quotes. This method should
   * be called from within the initialization list of the object inheriting the
   * MooseParsedFunctionInterface
   * @param function_str The name of the ParsedFunction
   * @return The vector of strings, if the input function is valid
   * @see ParsedFunction
   */
  const std::string verifyFunction(const std::string & function_str);

  /// Reference to the FEProblemBase class for this object
  FEProblemBase & _pfb_feproblem;

  /// Variables passed to libMesh::ParsedFunction
  const std::vector<std::string> _vars;

  /// Values passed by the user, they may be Reals for Postprocessors
  const std::vector<std::string> _vals;

  /// Pointer to the Parsed function wrapper object for the scalar
  std::unique_ptr<MooseParsedFunctionWrapper> _function_ptr;
};
