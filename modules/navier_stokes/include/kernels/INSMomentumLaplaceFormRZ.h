/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);
};

#endif
