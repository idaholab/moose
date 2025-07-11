//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "TimePeriodBase.h"
#include "AnalysisStepUOInterface.h"

class AnalysisStepUserObject;

/**
 * A basic control for disabling objects for a portion of the simulation based on the analysis step
 * concept.
 */
class AnalysisStepPeriod : public TimePeriodBase, public AnalysisStepUOInterface
{
public:
  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  static InputParameters validParams();

  AnalysisStepPeriod(const InputParameters & parameters);

protected:
  /**
   * If enabled, this injects the start/end times into the TimeStepper sync times.
   */
  void initialSetup() override;

  virtual bool conditionMet(const unsigned int & i) override;

  const AnalysisStepUserObject * _step_user_object;
};
