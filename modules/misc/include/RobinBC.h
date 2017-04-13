/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef ROBINBC_H
#define ROBINBC_H

#include "IntegratedBC.h"

class RobinBC;

template <>
InputParameters validParams<RobinBC>();

class RobinBC : public IntegratedBC
{
public:
  RobinBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  Real _value;
};

#endif // ROBINBC_H
