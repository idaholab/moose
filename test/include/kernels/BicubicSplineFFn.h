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

#ifndef BICUBICSPLINEFFN_H
#define BICUBICSPLINEFFN_H

#include "Kernel.h"
#include "BicubicSplineFunction.h"

class BicubicSplineFFn;

template<>
InputParameters validParams<BicubicSplineFFn>();

/**
 * Forcing function defined with a spline
 */
class BicubicSplineFFn : public Kernel
{
public:
  BicubicSplineFFn(const InputParameters & parameters);
  virtual ~BicubicSplineFFn();

protected:
  virtual Real computeQpResidual();

  BicubicSplineFunction & _fn;
};


#endif /* BICUBICSPLINEFFN_H */
