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

#ifndef SCALARDIRICHLETBC_H
#define SCALARDIRICHLETBC_H

#include "NodalBC.h"

//Forward Declarations
class ScalarDirichletBC;

template<>
InputParameters validParams<ScalarDirichletBC>();

/**
 * Implements a Dirichlet BC where scalar variable is coupled in
 */
class ScalarDirichletBC : public NodalBC
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  ScalarDirichletBC(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  /**
   * Holds the values of a coupled scalar variable.
   */
  VariableValue & _scalar_val;
};

#endif // SCALARDIRICHLETBC_H
