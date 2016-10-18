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

#ifndef AB2PREDICTORCORRECTOR_H
#define AB2PREDICTORCORRECTOR_H

// MOOSE includes
#include "TimeStepper.h"

// C++ includes
#include <fstream>

// Forward Declarations
class AB2PredictorCorrector;

template<>
InputParameters validParams<AB2PredictorCorrector>();

/**
 * A TimeStepper based on the AB2 method.  Increases the timestep if
 * the difference between the actual and AB2-predicted solutions is
 * small enough.
 */
class AB2PredictorCorrector : public TimeStepper
{
public:
  AB2PredictorCorrector(const InputParameters & parameters);

  virtual Stepper * buildStepper() override;

protected:
  /// error tolerance
  Real _e_tol;
  Real _e_max;
  /// maximum increase ratio
  Real _max_increase;
  /// steps to take before increasing dt
  int _steps_between_increase;
  int _start_adapting;
  /// scaling_parameter for time step selection, default is 0.8
  Real _scaling_parameter;
};

#endif // AB2PREDICTORCORRECTOR_H
