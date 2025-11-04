//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 *  Computes the heating due to the electic field (E) in the form of
 *  0.5 Re( J * E^* )
 *  where J is the current and E^* is the complex conjugate of the electric field.
 */
class SourceCurrentHeating : public AuxKernel
{
public:
  static InputParameters validParams();

  SourceCurrentHeating(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// Vector variable of the real component of the electric field
  const VectorVariableValue & _E_real;

  /// Vector variable of the imaginary component of the electric field
  const VectorVariableValue & _E_imag;

  /// Real component of the current source
  const Function & _source_real;

  /// Imaginary component of the current source
  const Function & _source_imag;
};
