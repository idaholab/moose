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
#include "Function.h"
#include "MooseParsedFunctionBase.h"

/**
 * This class is used to evaluate symbolic equations passed in to Moose through
 * the input file. It supports symbolic variables that you can change by putting
 * a different value in a reference returned by getVarAddr().
 *
 * Documentation for the Function Parser can be found at:
 * http://warp.povusers.org/FunctionParser/fparser.html
 */
template <typename T>
class MooseParsedFunctionTempl : public T, public MooseParsedFunctionBase
{
public:
  /**
   * Created from MooseSystem via the FunctionFactory.
   * @param parameters The input parameters
   */
  static InputParameters validParams();

  MooseParsedFunctionTempl(const InputParameters & parameters);

  using T::value;
  /**
   * Evaluate the equation at the given location. For 1-D and 2-D equations
   * x and y are optional.
   * @param t The evaluation time
   * @param pt The current point (x,y,z)
   * @return The result of evaluating the function
   */
  virtual Real value(Real t, const Point & pt) const override;

  using T::gradient;
  /**
   * Evaluate the gradient of the function. This is computed in libMesh
   * through automatic symbolic differentiation.
   */
  virtual RealGradient gradient(Real t, const Point & p) const override;

  /**
   * Evaluate the time derivative of the function. This is computed in libMesh
   * through automatic symbolic differentiation.
   * \param t The time
   * \param p The point in space (x,y,z)
   * \return The time derivative of the function at the specified time and location
   */
  virtual Real timeDerivative(Real t, const Point & p) const override;

  /**
   * Method invalid for ParsedGradFunction
   * @see ParsedVectorFunction
   */
  virtual RealVectorValue vectorValue(Real t, const Point & p) const override;

  /**
   * Creates the parsed function.
   */
  virtual void initialSetup() override;

protected:
  /// The function defined by the user
  std::string _value;

  friend class ParsedFunctionTest;
};

typedef MooseParsedFunctionTempl<FunctionTempl<ADReal>> ADMooseParsedFunction;

class MooseParsedFunction : public MooseParsedFunctionTempl<Function>
{
public:
  static InputParameters validParams() { return MooseParsedFunctionTempl<Function>::validParams(); }
  MooseParsedFunction(const InputParameters & params) : MooseParsedFunctionTempl<Function>(params)
  {
  }
};
