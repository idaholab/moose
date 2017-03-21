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
#ifndef MMSCOUPLEDDIRICHLETBC_H_
#define MMSCOUPLEDDIRICHLETBC_H_

#include "NodalBC.h"

class MMSCoupledDirichletBC;

template <>
InputParameters validParams<MMSCoupledDirichletBC>();

class MMSCoupledDirichletBC : public NodalBC
{
public:
  MMSCoupledDirichletBC(const InputParameters & parameters);
  virtual ~MMSCoupledDirichletBC() {}

protected:
  virtual Real computeQpResidual();

  Real _value; // Multiplier on the boundary
  unsigned int _mesh_dimension;
};

#endif // MMSCOUPLEDDIRICHLETBC_H_
