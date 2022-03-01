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
 * This class is similar to ParsedFunction except it also supports returning the
 * gradient of the function.
 *
 * Documentation for the Function Parser can be found at:
 * http://warp.povusers.org/FunctionParser/fparser.html
 */
class MooseParsedGradFunction : public Function, public MooseParsedFunctionBase
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters
   */
  static InputParameters validParams();

  MooseParsedGradFunction(const InputParameters & parameters);

  /**
   * Destructor necessary for std::unique_ptr usage
   */
  virtual ~MooseParsedGradFunction();

  using Function::value;
  /**
   * Return a scalar value from the function
   * @param t Current time
   * @param p The current spatial location
   */
  virtual Real value(Real t, const Point & p) const override;

  /**
   * Compute the gradient of the function
   * @param t The current time
   * @param p The current point (x,y,z)
   * @return Gradient of the function
   */
  virtual RealGradient gradient(Real t, const Point & p) const override;

  /**
   * Method invalid for ParsedGradFunction
   * @see ParsedVectorFunction
   */
  virtual RealVectorValue vectorValue(Real t, const Point & p) const override;

  /**
   * Creates two libMesh::ParsedFunction objects for returning a vector via the 'gradient' method
   * and a scalar vis the 'value' method
   */
  virtual void initialSetup() override;

protected:
  /// String for the scalar function string
  std::string _value;

  /// String for the gradient, vector function string
  std::string _grad_value;

  /// Pointer to the Parsed function wrapper object for the gradient
  std::unique_ptr<MooseParsedFunctionWrapper> _grad_function_ptr;
};
