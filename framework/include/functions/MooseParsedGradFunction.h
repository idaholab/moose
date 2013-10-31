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

#ifndef MOOSEPARSEDGRADFUNCTION_H
#define MOOSEPARSEDGRADFUNCTION_H

// MOOSE includes
#include "Function.h"
#include "MooseParsedFunctionBase.h"
#include "MooseParsedFunctionWrapper.h"

// Forward declerations
class MooseParsedGradFunction;

template<>
InputParameters validParams<MooseParsedGradFunction>();

/**
 * This class is similar to ParsedFunction except it also supports returning the
 * gradient of the function.
 *
 * Documentation for the Function Parser can be found at:
 * http://warp.povusers.org/FunctionParser/fparser.html
 */
class MooseParsedGradFunction :
  public Function,
  public MooseParsedFunctionBase
{
public:
  /**
   * Class constructor
   * @param name The name of the function
   * @param parameters The input parameters
   */
  MooseParsedGradFunction(const std::string & name, InputParameters parameters);

  /**
   * Class destructor
   */
  virtual ~MooseParsedGradFunction();

  /**
   * Return a scalar value from the function
   * @param t Current time
   * @param p The current spatial location
   */
  virtual Real value(Real t, const Point & p);

  /**
   * Compute the gradient of the function
   * @param t The current time
   * @param pt The current point (x,y,z)
   * @return Gradient of the function
   */
  virtual RealGradient gradient(Real t, const Point & p);

  /**
   * Method invalid for ParsedGradFunction
   * @see ParsedVectorFunction
   */
  virtual RealVectorValue vectorValue(Real t, const Point & p);

  /**
   * Creates two libMesh::ParsedFunction objects for returning a vector via the 'gradient' method
   * and a scalar vis the 'value' method
   */
  virtual void initialSetup();

protected:

  /// String for the scalar function string
  std::string _value;

  /// String for the gradient, vector function string
  std::string _grad_value;

  /// Pointer to the Parsed function wrapper object for the scalar
  MooseParsedFunctionWrapper * _function_ptr;

  /// Pointer to the Parsed function wrapper object for the gradient
  MooseParsedFunctionWrapper * _grad_function_ptr;
};

#endif // MOOSEPARSEDGRADFUNCTION_H
