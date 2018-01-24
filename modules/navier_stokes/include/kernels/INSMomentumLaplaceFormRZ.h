//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSMOMENTUMLAPLACEFORMRZ_H
#define INSMOMENTUMLAPLACEFORMRZ_H

#include "INSMomentumLaplaceForm.h"

// Forward Declarations
class INSMomentumLaplaceFormRZ;

template <>
InputParameters validParams<INSMomentumLaplaceFormRZ>();

/**
 * This class computes additional momentum equation residual and
 * Jacobian contributions for the incompressible Navier-Stokes
 * momentum equation in RZ (axisymmetric cylindrical) coordinates,
 * using the "Laplace" form of the governing equations.
 */
class INSMomentumLaplaceFormRZ : public INSMomentumLaplaceForm
{
public:
  INSMomentumLaplaceFormRZ(const InputParameters & parameters);

  virtual ~INSMomentumLaplaceFormRZ() {}

protected:
  virtual RealVectorValue strongViscousTermLaplace() override;
  virtual RealVectorValue dStrongViscDUCompLaplace(unsigned comp) override;
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned jvar) override;
};

#endif
