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

#ifndef MOOSEPARSEDVECTORFUNCTION_H
#define MOOSEPARSEDVECTORFUNCTION_H

// MOOSE includes
#include "Function.h"
#include "MooseParsedFunctionBase.h"
#include "MooseParsedFunctionWrapper.h"

// Forward decleratrions
class MooseParsedVectorFunction;

template<>
InputParameters validParams<MooseParsedVectorFunction>();

/**
 * This class is similar to ParsedFunction except it returns a vector function
 *
 */
class MooseParsedVectorFunction :
  public Function,
  public MooseParsedFunctionBase
{
public:
  /**
   * Class constructor
   * @param name The name of the function
   * @param parameters The input parameters
   */
  MooseParsedVectorFunction(const std::string & name, InputParameters parameters);

  /**
   * Class destructor
   */
  virtual ~MooseParsedVectorFunction();

  /**
   * Returns the vector function evaluation
   * @param t The time
   * @param p The current x,y,z location
   * @return The vector resulting from evaluating the function
   */
  virtual RealVectorValue vectorValue(Real t, const Point & p);

  /**
   * Gradient method is not valid in a vector function
   * \see ParsedGradFunction
   */
  virtual RealGradient gradient(Real t, const Point & p);

  /**
   * Creates the libMesh::ParsedFunction for returning a vector via the 'vectorValue' method
   */
  virtual void initialSetup();

protected:

  /// Storage for gradient input function(s), in format ready for libMesh
  std::string _vector_value;

  // Pointer to the wrapper object for parsed functions
  MooseParsedFunctionWrapper * _function_ptr;

};

#endif //MOOSEPARSEDVECTORFUNCTION_H
