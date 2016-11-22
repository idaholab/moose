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

#ifndef PIECEWISESTEPPER_H
#define PIECEWISESTEPPER_H

#include "Stepper.h"
#include "LinearInterpolation.h"

class PiecewiseStepper;

template<>
InputParameters validParams<PiecewiseStepper>();

/**
 * Choose dt based on a list of dt's and times
 */
class PiecewiseStepper : public Stepper
{
public:
  PiecewiseStepper(const InputParameters & parameters);
  virtual ~PiecewiseStepper();

  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

  virtual Real computeFailedDT() override;

protected:
  const std::vector<Real> & _times;
  const std::vector<Real> & _dts;

  /// Whether or not to interpolate DT between times
  bool _interpolate;

  /// Whether or not to perfectly land on the given times
  bool _sync_to_times;

  LinearInterpolation _linear_interpolation;
};

#endif /* PIECEWISESTEPPER_H */
