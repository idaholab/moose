/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMPSPG_H
#define INSMOMENTUMPSPG_H

#include "INSBase.h"

// Forward Declarations
class INSMomentumPSPG;

template <>
InputParameters validParams<INSMomentumPSPG>();

/**
 * This class computes the mass equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation.
 */
class INSMomentumPSPG : public INSBase
{
public:
  INSMomentumPSPG(const InputParameters & parameters);

  virtual ~INSMomentumPSPG() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  const Real & _alpha;
  bool _consistent;
  const VariableTestSecond & _second_test;
  unsigned _component;
};

#endif // INSMOMENTUMPSPG_H
