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

#ifndef DIRICHLETBC_H
#define DIRICHLETBC_H

#include "NodalBC.h"

class DirichletBC : public NodalBC
{
public:
  DirichletBC(const std::string & name, InputParameters parameters);
  virtual ~DirichletBC();

protected:
  virtual Real computeQpResidual();

  Real _value;
};

template<>
InputParameters validParams<DirichletBC>();

#endif /* DIRICHLETBC_H */
