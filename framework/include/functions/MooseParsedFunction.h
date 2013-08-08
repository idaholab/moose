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

#ifndef MOOSEPARSEDFUNCTION_H
#define MOOSEPARSEDFUNCTION_H

#include "Function.h"

#include <iostream>
#include <string>
#include <map>

#include "libmesh/parsed_function.h"

class MooseParsedFunction;

template<>
InputParameters validParams<MooseParsedFunction>();

/**
 * This class is used to evaluate symbolic equations passed in to Moose through
 * the input file. It supports symbolic variables that you can change by putting
 * a different value in a reference returned by getVarAddr().
 *
 * Documentation for the Function Parser can be found at:
 * http://warp.povusers.org/FunctionParser/fparser.html
 *
 * This class is currently NOT thread safe. If you are going to be evaluating
 * the same functor simultaneously you must compile fparser in thread safe mode.
 * (Do this probably by editing the fconfig.h file)
 */
class MooseParsedFunction : public Function
{
public:
  /**
   * Created from MooseSystem via the FunctionFactory.
   * @param name The name of the function
   * @param parameters The input parameters
   */
  MooseParsedFunction(const std::string & name, InputParameters parameters);

  /**
   * Destructor, it cleans up the libMesh::ParsedFunction object
   */
  virtual ~MooseParsedFunction();

  /**
   * Get the address to stick the value of the specified variable in. When you
   * evaluate the functor it uses the passed values for t, x, y, and z, and
   * whatever value you put in the address returned by this method for every
   * other symbolic variables.
   *
   * This method should only need to be called once for each variable when you
   * initialize your Kernel/BC/whatever. The address will stay the same for
   * the entire computation.
   *
   * @param var the name of the variable passed in the constructor.
   */
  inline Real & getVarAddr(const std::string & var) { return _function->getVarAddress(var); }

  /**
   * Evaluate the equation at the given location. For 1-D and 2-D equations
   * x and y are optional.
   * @param t The evaluation time
   * @param pt The current point (x,y,z)
   * @return The result of evaluating the function
   */
  virtual Real value(Real t, const Point & pt);

  /**
   * Method for defining the libMesh::ParsedFunction
   */
  virtual void initialSetup();

protected:
  /**
   * Override this method if you want to make your own MooseParsedFunction with custom
   * constants available to the end user. In the base class pi and e are
   * defined. This method is called when the FunctionParser object is being
   * initialized in initializeParser before the function is parsed.
   */
  // TODO: This function is currently not implemented
  // void defineConstants(FunctionParser & fp);

  /**
   * This method just checks to see if the function value contains quotes
   * @param name The name of the ParsedFunction
   * @param value The function given as a std::string
   * @return The vector of strings, if the input function is valid
   */
  const std::string verifyInput(const std::string & name, const std::string & value);

  /**
   * This method verifies that none of the implied variables are being re-declared as
   * variables in the parsed function.
   * @param name The name of the ParsedFunction
   * @param vars A vector of variables (std::strings) that are given in the function
   * @return The variables (i.e., vars)
   * TODO: The syntax between verifyInput and verifyVars should be consistent, which will effect MooseParsedGradFunction
   */
  const std::vector<std::string> verifyVars(const std::string & name, const std::vector<std::string> & vars);

  /**
   * A method for updating the Postprocessor values of the libMesh::ParsedFunction from the values in the
   * Postprocessors
   */
  virtual void updatePostprocessorValues();

  /// The function defined by the user
  std::string _value;

  /// Variables passed to libMesh::ParsedFunction, see initialSetup()
  const std::vector<std::string> _vars;

  /// Values passed by the user, they may be Reals for Postprocessors
  const std::vector<std::string> _input_vals;

  /// Initial values of variables passed to libMesh::ParsedFunction
  std::vector<Real> _vals;

  /// Vector of pointers to PP values
  std::vector<Real *> _pp_vals;

  /// Vector of pointers to the variables in libMesh::ParseFunction
  std::vector<Real *> _var_addr;

  /// Pointer to libMesh::ParsedFunction
  ParsedFunction<Real> * _function;

  /// Stores the relative location of variables (in _vars) that are connected to Postprocessors
  std::vector<unsigned int> _pp_index;

  /// Initialization flag
  bool _initialized;
};
#endif //MOOSEPARSEDFUNCTION_H
