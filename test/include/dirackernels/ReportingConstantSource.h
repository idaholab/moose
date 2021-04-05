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
#include "DiracKernel.h"

/**
 * A test class that uses a AuxScalarVariable to share with another
 * kernel as well as report the value via a postprocessor
 */
class ReportingConstantSource : public DiracKernel
{
public:
  static InputParameters validParams();

  ReportingConstantSource(const InputParameters & parameters);
  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  const VariableValue & _shared_var;
  const std::vector<Real> _point_param;
  Point _p;
  const Real _factor;
};
