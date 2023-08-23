//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HSBoundary.h"

/**
 * Heat structure boundary condition to apply a heat flux transferred from
 * another application.
 */
class HSBoundaryExternalAppHeatFlux : public HSBoundary
{
public:
  static InputParameters validParams();

  HSBoundaryExternalAppHeatFlux(const InputParameters & params);

  virtual void check() const override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// Heat flux variable name
  const VariableName & _heat_flux_name;
  /// External app perimeter post-processor name
  const PostprocessorName & _perimeter_ext_pp_name;
};
