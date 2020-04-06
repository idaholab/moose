//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DiracKernel.h"

/**
 * Point source (or sink) that adds (removes) fluid at a constant mass flux rate for times
 * between the specified start and end times. If no start and end times are specified,
 * the source (sink) starts at the start of the simulation and continues to act indefinitely
 */
class PorousFlowSquarePulsePointSource : public DiracKernel
{
public:
  static InputParameters validParams();

  PorousFlowSquarePulsePointSource(const InputParameters & parameters);

  virtual void addPoints() override;
  virtual Real computeQpResidual() override;

protected:
  /// The constant mass flux (kg/s)
  const Real _mass_flux;

  /// The location of the point source (sink)
  const Point _p;

  /// The time at which the point source (sink) starts operating
  const Real _start_time;

  /// The time at which the point source (sink) stops operating
  const Real _end_time;
};
