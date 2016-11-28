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

#ifndef DT2STEPPER_H
#define DT2STEPPER_H

#include "Stepper.h"

class DT2Stepper;

template<>
InputParameters validParams<DT2Stepper>();

/**
 * An adaptive timestepper that compares the solution obtained from a
 * single step of size dt with two steps of size dt/2 and adjusts the
 * next timestep accordingly.
 */
class DT2Stepper : public Stepper
{
public:
  DT2Stepper(const InputParameters & parameters);

  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

  virtual Real computeFailedDT() override;

private:
  Real windowDT();
  Real resetWindow(Real start, Real dt);
  Real calculateError();

  const Real & _input_dt;
  const Real & _e_tol;
  const Real & _e_max;
  const Real & _max_increase;

  Real _tol;
  int _order;

  Real _start_time;
  Real _end_time;

  std::unique_ptr<NumericVector<Number>> _big_soln;
};

#endif /* DT2STEPPER_H */
