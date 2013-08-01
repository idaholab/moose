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

#ifndef SPLINEFFN_H
#define SPLINEFFN_H

#include "Kernel.h"
#include "SplineFunction.h"

class SplineFFn;

template<>
InputParameters validParams<SplineFFn>();

/**
 * Forcing function defined with a spline
 */
class SplineFFn : public Kernel
{
public:
  SplineFFn(const std::string & name, InputParameters parameters);
  virtual ~SplineFFn();

protected:
  virtual Real computeQpResidual();

  SplineFunction & _fn;
};


#endif /* SPLINEFFN_H */
