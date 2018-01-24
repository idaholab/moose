/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMASS_H
#define INSMASS_H

#include "INSBase.h"

// Forward Declarations
class INSMass;

template <>
InputParameters validParams<INSMass>();

/**
 * This class computes the mass equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation.
 */
class INSMass : public INSBase
{
public:
  INSMass(const InputParameters & parameters);

  virtual ~INSMass() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  virtual Real computeQpPGResidual();
  virtual Real computeQpPGJacobian();
  virtual Real computeQpPGOffDiagJacobian(unsigned comp);

  bool _pspg;
  Function & _x_ffn;
  Function & _y_ffn;
  Function & _z_ffn;
};

#endif // INSMASS_H
