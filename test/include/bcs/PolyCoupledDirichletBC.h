/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#ifndef POLYCOUPLEDDIRICHLETBC_H
#define POLYCOUPLEDDIRICHLETBC_H

#include "NodalBC.h"

class PolyCoupledDirichletBC;

template <>
InputParameters validParams<PolyCoupledDirichletBC>();

class PolyCoupledDirichletBC : public NodalBC
{
public:
  PolyCoupledDirichletBC(const InputParameters & parameters);

  virtual ~PolyCoupledDirichletBC() {}

protected:
  virtual Real computeQpResidual();

  Real _value; // Multiplier on the boundary
};

#endif // POLYCOUPLEDDIRICHLETBC_H
