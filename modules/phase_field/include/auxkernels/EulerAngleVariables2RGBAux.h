/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef EULERANGLEVARIABLES2RGBAUX_H
#define EULERANGLEVARIABLES2RGBAUX_H

#include "AuxKernel.h"

class EulerAngleVariables2RGBAux;

template <>
InputParameters validParams<EulerAngleVariables2RGBAux>();

/**
 * Create an encoded RGB triplet from Euler angle data.
 * The color value is encoded as (R*256+G)*256+B with R,G, and B ranging
 * from 0..255.
 */
class EulerAngleVariables2RGBAux : public AuxKernel
{
public:
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

#endif // EULERANGLEVARIABLES2RGBAUX_H
