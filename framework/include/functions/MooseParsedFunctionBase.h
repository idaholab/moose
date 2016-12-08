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

#ifndef MOOSEPARSEDFUNCTIONBASE_H
#define MOOSEPARSEDFUNCTIONBASE_H

// Standard library
#include <vector>
#include <memory>

// MOOSE includes
#include "InputParameters.h"
#include "MooseError.h"

// Forward declarations
class MooseParsedFunctionBase;
class MooseParsedFunctionWrapper;

/**
 * Creates the 'vars' and 'vals' parameters used by all ParsedFunctions, the
 * parameters provided from this function should be appeneded to the parameters
 * for the class using the += operator.
 * @see MooseParsedFunction,  MooseParsedGradFunction, MooseParsedVectorFunction
 */
template<>
InputParameters validParams<MooseParsedFunctionBase>();

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
  MooseParsedFunctionBase(const InputParameters & parameters);

  /**
   * Class destructor.
   */
  virtual ~MooseParsedFunctionBase();

protected:

   /**
   * A helper method to check if the function value contains quotes. This method should
   * be called from within the initialization list of the object inheriting the MooseParsedFunctionInterface
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

#endif // MOOSEPARSEDFUNCTIONBASE_H
