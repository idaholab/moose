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

#ifndef TIMEDERIVATIVE_H
#define TIMEDERIVATIVE_H

#include "TimeKernel.h"

// Forward Declaration
class TimeDerivative;

template<>
InputParameters validParams<TimeDerivative>();

class TimeDerivative : public TimeKernel
{
public:

  TimeDerivative(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
protected:
  virtual Real computeQpResidual();
  
  virtual Real computeQpJacobian();

  /**
   * Coefficients (weights) for the BDF2 time discretization.
   */
  std::vector<Real> & _time_weight;
};

#endif //TIMEDERIVATIVE
