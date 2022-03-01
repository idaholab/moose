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
  static InputParameters validParams();

  MooseParsedVectorFunction(const InputParameters & parameters);

  virtual RealVectorValue vectorValue(Real t, const Point & p) const override;

  virtual RealVectorValue vectorCurl(Real t, const Point & p) const override;

  virtual RealGradient gradient(Real t, const Point & p) const override;

  virtual void initialSetup() override;

protected:
  /// Storage for gradient, vector input function(s), in format ready for libMesh
  std::string _vector_value;

  /// Storage for gradient, curl input function(s), in format ready for libMesh
  std::string _curl_value;

  /// Pointer to the Parsed function wrapper object for the curl
  std::unique_ptr<MooseParsedFunctionWrapper> _curl_function_ptr;
};
