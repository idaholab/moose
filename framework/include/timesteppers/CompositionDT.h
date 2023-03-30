//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeStepper.h"

class CompositionDT : public TimeStepper
{
public:
  static InputParameters validParams();

  static MooseEnum getCompositionTypes()
  {
    return MooseEnum("max min average limiting", "average");
  }

  CompositionDT(const InputParameters & parameters);

  Real produceCompositionDT();

  Real maxTimeStep();

  Real minTimeStep();

  Real averageTimeStep();

  Real limitingTimeStep();

protected:
  //  virtual void init() override;
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

private:
  MooseEnum _composition_type;

  const std::vector<std::string> _inputs;

  Real _dt;

  Real _has_initial_dt;

  Real _initial_dt;

  std::map<std::string, Real> _dts;
};
