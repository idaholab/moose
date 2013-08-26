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

#ifndef BNDTESTDIRICHLETBC_H
#define BNDTESTDIRICHLETBC_H

#include "NodalBC.h"

class BndTestDirichletBC;

template<>
InputParameters validParams<BndTestDirichletBC>();

/**
 * Boundary condition of a Dirichlet type
 *
 * Sets the value in the node
 */
class BndTestDirichletBC : public NodalBC
{
public:
  BndTestDirichletBC(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  /// The value for this BC
  const Real & _value;
};

#endif /* BNDTESTDIRICHLETBC_H */
