/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATHFREEENERGY_H
#define MATHFREEENERGY_H

#include "DerivativeFunctionMaterialBase.h"

// Forward Declarations
class MathFreeEnergy;

template <>
InputParameters validParams<MathFreeEnergy>();

/**
 * Material class that creates the math free energy and its derivatives
 * for use with CHParsed and SplitCHParsed. \f$ F = \frac14(1 + c)^2(1 - c)^2 \f$.
 */
class MathFreeEnergy : public DerivativeFunctionMaterialBase
{
public:
  MathFreeEnergy(const InputParameters & parameters);

protected:
  virtual Real computeF();
  virtual Real computeDF(unsigned int j_var);
  virtual Real computeD2F(unsigned int j_var, unsigned int k_var);
  virtual Real computeD3F(unsigned int j_var, unsigned int k_var, unsigned int l_var);

private:
  /// Coupled variable value for the concentration \f$ c \f$.
  const VariableValue & _c;
  unsigned int _c_var;
};

#endif // MATHFREEENERGY_H
