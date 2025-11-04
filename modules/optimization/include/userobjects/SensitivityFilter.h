//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "RadialAverage.h"

/**
 * Element user object that filters the objective function sensitivities via a radial average user
 * objects. This object can be used to apply a Solid Isotropic Material Penalization (SIMP) to
 * optimization.
 */
class SensitivityFilter : public ElementUserObject
{
public:
  static InputParameters validParams();

  SensitivityFilter(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override{};
  virtual void threadJoin(const UserObject &) override{};

protected:
  /// Radial average user object
  const RadialAverage::Result & _filter;
  /// Sensitivity with respect to density
  MooseWritableVariable & _density_sensitivity;
  /// Pseudo-density variable name
  const VariableName _design_density_name;
  /// The pseudo-density variable
  const MooseVariable & _design_density;
};
