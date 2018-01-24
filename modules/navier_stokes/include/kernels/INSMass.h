//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
