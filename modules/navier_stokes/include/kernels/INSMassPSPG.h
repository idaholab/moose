/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMASSPSPG_H
#define INSMASSPSPG_H

#include "INSBase.h"

// Forward Declarations
class INSMassPSPG;

template <>
InputParameters validParams<INSMassPSPG>();

/**
 * This class computes the mass equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation.
 */
class INSMassPSPG : public INSBase
{
public:
  INSMassPSPG(const InputParameters & parameters);

  virtual ~INSMassPSPG() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  const Real & _alpha;
  bool _consistent;
};

#endif // INSMASSPSPG_H
