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

#ifndef FUNCTIONDIRICHLETBC_H
#define FUNCTIONDIRICHLETBC_H

#include "BoundaryCondition.h"

//Forward Declarations
class FunctionDirichletBC;
class Function;

template<>
InputParameters validParams<FunctionDirichletBC>();

/**
 * Defines a boundary condition that forces the value to be a user specified
 * function at the boundary.
 */
class FunctionDirichletBC : public BoundaryCondition
{
public:

  FunctionDirichletBC(const std::string & name,
             InputParameters parameters);

protected:
  /**
   * Evaluate the function at the current quadrature point and timestep.
   */
  Real f();

  /**
   * returns (u - the function)
   */
  virtual Real computeQpResidual();

private:
  Function & _func;
};
#endif //FUNCTIONDIRICHLETBC_H
