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

#ifndef FUNCTIONIC_H
#define FUNCTIONIC_H

#include "InitialCondition.h"

//Forward Declarations
class FunctionIC;
class Function;

template<>
InputParameters validParams<FunctionIC>();

/**
 * Defines a boundary condition that forces the value to be a user specified
 * function at the boundary.
 */
class FunctionIC : public InitialCondition
{
public:

  FunctionIC(const std::string & name,
             InputParameters parameters);

protected:
  /**
   * Evaluate the function at the current quadrature point and timestep.
   */
  Real f();

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */
  virtual Real value(const Point &p);

private:
  Function & _func;
};
#endif //FUNCTIONIC_H
