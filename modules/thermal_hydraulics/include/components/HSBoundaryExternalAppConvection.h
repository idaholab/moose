//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HSBoundary.h"

/**
 * Heat structure boundary condition to perform convective heat transfer with an external
 * application
 */
class HSBoundaryExternalAppConvection : public HSBoundary
{
public:
  HSBoundaryExternalAppConvection(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// Temperature from external application
  const VariableName & _T_ext_var_name;
  /// Heat transfer coefficient from external application
  const VariableName & _htc_ext_var_name;

public:
  static InputParameters validParams();
};
