//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"

/**
 * Test postprocessor that integrates the normal gradient of one component of a coupled array
 * variable over a boundary, using the automatic differentiation gradient on the element face.
 */
class ADArraySideFluxIntegral : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  ADArraySideFluxIntegral(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// AD gradient of the coupled array variable
  const ADArrayVariableGradient & _grad_v;
  /// array component to integrate
  const unsigned int _component;
};
