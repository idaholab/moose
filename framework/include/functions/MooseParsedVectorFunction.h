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
   * @param parameters The input parameters
   */
  MooseParsedVectorFunction(const InputParameters & parameters);

  virtual RealVectorValue vectorValue(Real t, const Point & p) override;

  virtual RealGradient gradient(Real t, const Point & p) override;

  virtual void initialSetup() override;

protected:

  /// Storage for gradient input function(s), in format ready for libMesh
  std::string _vector_value;
};

#endif //MOOSEPARSEDVECTORFUNCTION_H
