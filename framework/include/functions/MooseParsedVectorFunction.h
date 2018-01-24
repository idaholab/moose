//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSEPARSEDVECTORFUNCTION_H
#define MOOSEPARSEDVECTORFUNCTION_H

// MOOSE includes
#include "Function.h"
#include "MooseParsedFunctionBase.h"

// Forward decleratrions
class MooseParsedVectorFunction;

template <>
InputParameters validParams<MooseParsedVectorFunction>();

/**
 * This class is similar to ParsedFunction except it returns a vector function
 *
 */
class MooseParsedVectorFunction : public Function, public MooseParsedFunctionBase
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

#endif // MOOSEPARSEDVECTORFUNCTION_H
