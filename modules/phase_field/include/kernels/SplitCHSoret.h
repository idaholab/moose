/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPLITCHSORET_H
#define SPLITCHSORET_H

#include "Kernel.h"

//Forward Declaration
class SplitCHSoret;

template<>
InputParameters validParams<SplitCHSoret>();
/**
 * SplitCHSoret adds the soret effect in the split form of the Cahn-Hilliard
 * equation.
 */
class SplitCHSoret : public Kernel
{
public:
  SplitCHSoret(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// int label for temperature variable
  unsigned int _T_var;

  /// Coupled variable for the temperature
  VariableValue & _T;

  /// Variable gradient for temperature
  VariableGradient & _grad_T;

  /// int label for the Concentration
  unsigned int _c_var;

  /// Variable value for the concentration
  VariableValue & _c;

  /// Diffusivity material property
  const MaterialProperty<Real> & _D;

  /// Heat of transport material property
  const MaterialProperty<Real> & _Q;

  /// Boltzmann constant
  const Real _kb;
};

#endif //SPLITCHSORET_H
