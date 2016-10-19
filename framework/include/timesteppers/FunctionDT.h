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

#ifndef FUNCTIONDT_H
#define FUNCTIONDT_H

#include "TimeStepper.h"
#include "LinearInterpolation.h"

class FunctionDT;

template<>
InputParameters validParams<FunctionDT>();

class FunctionDT : public TimeStepper
{
public:
  FunctionDT(const InputParameters & parameters);

  virtual Stepper * buildStepper() override;

private:
  const std::vector<Real> & _time_t;
  const std::vector<Real> & _time_dt;
  Real _growth_factor;
  Real _min_dt;
  /// Whether or not to interpolate DT between times
  bool _interpolate;
};

#endif /* FUNCTIONDT_H_ */
