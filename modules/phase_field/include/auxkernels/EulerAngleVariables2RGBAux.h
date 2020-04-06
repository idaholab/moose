//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Create an encoded RGB triplet from Euler angle data.
 * The color value is encoded as (R*256+G)*256+B with R,G, and B ranging
 * from 0..255.
 */
class EulerAngleVariables2RGBAux : public AuxKernel
{
public:
  static InputParameters validParams();

  EulerAngleVariables2RGBAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

private:
  /// Reference direction of the sample
  const unsigned int _sd;

  /// Type of value to be outputted
  const unsigned int _output_type;

  ///@{ Euler angles to visualize
  const VariableValue & _phi1;
  const VariableValue & _phi;
  const VariableValue & _phi2;
  ///@}

  /// EBSD Phase index
  const VariableValue & _phase;

  /// EBSD Crystal symmetry identifier
  const VariableValue & _sym;
};
