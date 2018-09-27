//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MASSFREEBC_H
#define MASSFREEBC_H

#include "IntegratedBC.h"

class MassFreeBC;

template <>
InputParameters validParams<MassFreeBC>();

/**
 *
 */
class MassFreeBC : public IntegratedBC
{
public:
  MassFreeBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const VariableValue & _vel_x;
  const VariableValue & _vel_y;
  const VariableValue & _vel_z;
};

#endif /* MASSFREEBC_H */
