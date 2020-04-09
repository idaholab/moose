//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSMass.h"

// Forward Declarations

/**
 * This class computes the mass equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation in RZ coordinates.  Inherits most of its functionality
 * from INSMass, and calls its computeQpXYZ() functions when
 * necessary.
 */
class INSMassRZ : public INSMass
{
public:
  static InputParameters validParams();

  INSMassRZ(const InputParameters & parameters);
  virtual ~INSMassRZ() {}

protected:
  virtual RealVectorValue strongViscousTermTraction() override;
  virtual RealVectorValue dStrongViscDUCompTraction(unsigned comp) override;
  virtual RealVectorValue strongViscousTermLaplace() override;
  virtual RealVectorValue dStrongViscDUCompLaplace(unsigned comp) override;
  virtual Real computeQpResidual() override;
  virtual Real computeQpOffDiagJacobian(unsigned jvar) override;
};
