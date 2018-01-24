/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMTRACTIONFORMRZ_H
#define INSMOMENTUMTRACTIONFORMRZ_H

#include "INSMomentumTractionForm.h"

// Forward Declarations
class INSMomentumTractionFormRZ;

template <>
InputParameters validParams<INSMomentumTractionFormRZ>();

/**
 * This class computes additional momentum equation residual and
 * Jacobian contributions for the incompressible Navier-Stokes
 * momentum equation in RZ (axisymmetric cylindrical) coordinates.
 */
class INSMomentumTractionFormRZ : public INSMomentumTractionForm
{
public:
  INSMomentumTractionFormRZ(const InputParameters & parameters);

  virtual ~INSMomentumTractionFormRZ() {}

protected:
  virtual RealVectorValue strongViscousTermTraction() override;
  virtual RealVectorValue dStrongViscDUCompTraction(unsigned comp) override;
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned jvar) override;
};

#endif
