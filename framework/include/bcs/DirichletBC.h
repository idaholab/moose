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

class DirichletBC;

template<>
InputParameters validParams<DirichletBC>();

/**
 * Boundary condition of a Dirichlet type
 *
 * Sets the value in the node
 */
class DirichletBC : public NodalBC
{
public:
  DirichletBC(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  Real _value;                                  /// the value for this BC
};

#endif /* DIRICHLETBC_H */
