/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMBASE_H
#define INSMOMENTUMBASE_H

#include "INSBase.h"

// Forward Declarations
class INSMomentumBase;

template <>
InputParameters validParams<INSMomentumBase>();

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation.
 */
class INSMomentumBase : public INSBase
{
public:
  INSMomentumBase(const InputParameters & parameters);

  virtual ~INSMomentumBase() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
  virtual Real computeQpResidualViscousPart() = 0;
  virtual Real computeQpJacobianViscousPart() = 0;
  virtual Real computeQpOffDiagJacobianViscousPart(unsigned jvar) = 0;

  virtual Real computeQpPGResidual();
  virtual Real computeQpPGJacobian(unsigned comp);

  unsigned _component;
  bool _integrate_p_by_parts;
  bool _supg;
  Function & _ffn;
};

#endif // INSMOMENTUMBASE_H
