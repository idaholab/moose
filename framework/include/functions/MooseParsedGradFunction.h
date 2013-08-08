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

#ifndef PARSEDGRADFUNCTION_H
#define PARSEDGRADFUNCTION_H

#include "Function.h"

#include "MooseParsedFunction.h"

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
class MooseParsedGradFunction : public MooseParsedFunction
{
public:
  /**
   * Class constructor
   * @param name The name of the function
   * @param parameters The input parameters
   */
  MooseParsedGradFunction(const std::string & name, InputParameters parameters);

  /**
   * Class destructor, it removes the libMesh::ParsedFunction
   */
  virtual ~MooseParsedGradFunction();

  /**
   * Compute the gradient of the function
   * @param t The current time
   * @param pt The current point (x,y,z)
   * @return Gradient of the function
   */
  virtual RealGradient gradient(Real t, const Point & pt);

  /**
   * Establishes a libMesh::ParsedFunction for the gradient
   */
  virtual void initialSetup();

protected:

  /**
   * A method for updating the Postprocessor values of the libMesh::ParsedFunction from the values in the
   * Postprocessors
   */
  virtual void updatePostprocessorValues();

  /// Storage for gradient input function(s), i.e., grad_x, grad_y, and grad_z, in format ready for libMesh
  std::string _grad_value;

  /// Pointer to the libMesh::ParsedFunction for the gradient
  ParsedFunction<Real> * _grad_function;

  /// Vector of pointers to the variables in libMesh::ParseFunctio
  std::vector<Real *> _grad_var_addr;

};

#endif //PARSEDGRADFUNCTION_H
