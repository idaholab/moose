/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSTEMPERATURENOBCBC_H
#define INSTEMPERATURENOBCBC_H

#include "IntegratedBC.h"

// Forward Declarations
class INSTemperatureNoBCBC;

template <>
InputParameters validParams<INSTemperatureNoBCBC>();

/**
 * This class implements the "No BC" boundary condition
 * discussed by Griffiths, Papanastiou, and others.
 */
class INSTemperatureNoBCBC : public IntegratedBC
{
public:
  INSTemperatureNoBCBC(const InputParameters & parameters);

  virtual ~INSTemperatureNoBCBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  Real _k;
};

#endif // INSTEMPERATURENOBCBC_H
