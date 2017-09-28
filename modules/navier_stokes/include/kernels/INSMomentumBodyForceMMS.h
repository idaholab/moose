/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMBODYFORCEMMS_H
#define INSMOMENTUMBODYFORCEMMS_H

#include "INSBase.h"

// Forward Declarations
class INSMomentumBodyForceMMS;

template <>
InputParameters validParams<INSMomentumBodyForceMMS>();

/**
 * This class computes the mms forcing function contribution to the SUPG component of the momentum
 * residual
 */
class INSMomentumBodyForceMMS : public INSBase
{
public:
  INSMomentumBodyForceMMS(const InputParameters & parameters);

  virtual ~INSMomentumBodyForceMMS() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  unsigned _component;
  Function & _ffn;
};

#endif // INSMOMENTUMBODYFORCEMMS_H
