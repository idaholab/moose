//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVPressureVariable.h"

class InputParameters;

class INSFVPressureNoQpComputation : public INSFVPressureVariable
{
public:
  INSFVPressureNoQpComputation(const InputParameters & params);

  /// Set a selection of routines that are use Qp values to error to make sure Qp values
  /// are not being computed. This is an optimization of INSFVVariables that we do not want to
  /// remove accidentally.
  virtual void requireQpComputations() const override
  {
    mooseError("Qp-calculations should not be requested");
  }
  inline const MooseArray<ADReal> & adDofValues() const override
  {
    mooseError("Qp-calculations should not be requested");
  }
  const DoFValue & dofValues() const override
  {
    mooseError("Qp-calculations should not be requested");
  }
  const DoFValue & dofValuesOld() const override
  {
    mooseError("Qp-calculations should not be requested");
  }
  const DoFValue & dofValuesDot() const override
  {
    mooseError("Qp-calculations should not be requested");
  }
};
