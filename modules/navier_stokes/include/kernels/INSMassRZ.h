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
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled values
  const VariableValue & _u_vel;
};

#endif // INSMASSRZ_H
