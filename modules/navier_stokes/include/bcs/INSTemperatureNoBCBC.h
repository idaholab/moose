//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

  const MaterialProperty<Real> & _k;
};

#endif // INSTEMPERATURENOBCBC_H
