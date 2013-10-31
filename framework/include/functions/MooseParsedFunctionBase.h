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
#include <map>

// MOOSE includes
#include "InputParameters.h"
#include "MooseError.h"

// Forward declarations
class MooseParsedFunctionBase;

/**
 * Creates the 'vars' and 'vals' parameters used by all ParsedFunctions, the
 * parameters provided from this function should be appeneded to the parameters
 * for the class using the += operator.
 * @see MooseParsedFunction,  MooseParsedGradFunction, MooseParsedVectorFunction
 */
template<>
InputParameters validParams<MooseParsedFunctionBase>();

/**
 * @class Adds user facing parameters for parsed function
 * @see ParsedFunction ParsedVectorFunction ParsedGradFunction
 */
class MooseParsedFunctionBase
{
public:

  /**
   * Class constructor for the interface.
   * @param name The name of the object (currently not used)
   * @param parameters Input parameters from the object, it must contain '_fe_problem'
   */
  MooseParsedFunctionBase(const std::string & /*name*/, InputParameters parameters);

  /**
   * Class destructor.
   */
  virtual ~MooseParsedFunctionBase();

protected:

   /**
   * A helper method to check if the function value contains quotes. This method should
   * be called from within the initialization list of the object inheriting the MooseParsedFunctionInterface
   * @param name The name of the ParsedFunction
   * @param value The function given as a std::string
   * @return The vector of strings, if the input function is valid
   * @see ParsedFunction
   */
   const std::string verifyFunction(const std::string & function_str);


  /// Reference to the FEProblem class for this object
  FEProblem & _pfb_feproblem;

  /// Variables passed to libMesh::ParsedFunction
  const std::vector<std::string> _vars;

  /// Values passed by the user, they may be Reals for Postprocessors
  const std::vector<std::string> _vals;

private:
  /**
   * Verifies that the 'vars' variable exists and that pre-defined variables are not used (i.e., x,y,z,t)
   * @param vars A vector of variables (std::strings) that are given in the function
   * @return The variables (i.e., vars)
   */
  const std::vector<std::string> verifyVars(const InputParameters & parameters);

  /**
   * Verifies that the 'vals' variable exists
   * @param vars A vector of values (std::strings) that are given in the function
   * @return The values (i.e., vals)
   */
  const std::vector<std::string> verifyVals(const InputParameters & parameters);
};

#endif // MOOSEPARSEDFUNCTIONBASE_H
