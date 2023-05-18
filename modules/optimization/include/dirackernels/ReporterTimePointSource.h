//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "ReporterPointSource.h"

/**
 * Apply a time dependent point load defined by Reporters.
 */
class ReporterTimePointSource : public ReporterPointSource
{
public:
  static InputParameters validParams();
  ReporterTimePointSource(const InputParameters & parameters);
  virtual void addPoints() override;
protected:
  /// time-coordinates from reporter
  const std::vector<Real> & _coordt;

private:
  /// The final time when we want to reverse the time index in function evaluation
  const Real & _reverse_time_end;

};
