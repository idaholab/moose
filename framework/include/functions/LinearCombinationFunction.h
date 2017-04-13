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

#ifndef LINEARCOMBINATIONFUNCTION_H
#define LINEARCOMBINATIONFUNCTION_H

#include "Function.h"
#include "FunctionInterface.h"

class LinearCombinationFunction;

template <>
InputParameters validParams<LinearCombinationFunction>();

/**
 * Sum_over_i (w_i * functions_i)
 */
class LinearCombinationFunction : public Function, protected FunctionInterface
{
public:
  LinearCombinationFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & pt) override;

private:
  std::vector<Real> _w;

  std::vector<Function *> _f;
};

#endif // LINEARCOMBINATIONFUNCTION_H
