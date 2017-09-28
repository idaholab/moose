/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMASSBODYFORCEMMS_H
#define INSMASSBODYFORCEMMS_H

#include "INSBase.h"

// Forward Declarations
class INSMassBodyForceMMS;

template <>
InputParameters validParams<INSMassBodyForceMMS>();

/**
 * This class computes the mms forcing function contribution to the PSPG component of the mass
 * residual
 */
class INSMassBodyForceMMS : public INSBase
{
public:
  INSMassBodyForceMMS(const InputParameters & parameters);

  virtual ~INSMassBodyForceMMS() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  Function & _x_ffn;
  Function & _y_ffn;
  Function & _z_ffn;
};

#endif // INSMASSBODYFORCEMMS_H
