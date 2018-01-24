/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMASSRZ_H
#define INSMASSRZ_H

#include "INSMass.h"

// Forward Declarations
class INSMassRZ;

template <>
InputParameters validParams<INSMassRZ>();

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

#endif // INSMASSRZ_H
