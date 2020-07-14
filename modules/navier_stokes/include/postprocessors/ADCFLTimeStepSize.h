//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes 
#include "ElementPostprocessor.h"

/**
 * Computes a time step size based on user-specified CFL number
 */
class ADCFLTimeStepSize : public ElementPostprocessor
{
public:
  ADCFLTimeStepSize(const InputParameters & parameters);

  virtual void execute() override;

  virtual void initialize() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// User-specified CFL number
  const Real _CFL;

  /// Velocity material property name(s)
  const std::vector<MaterialPropertyName> & _vel_names;
  /// Sound speed material property name(s)
  const std::vector<MaterialPropertyName> & _c_names;

  /// Number of phases
  const unsigned int _n_phases;

  /// Velocity material properties
  std::vector<const ADMaterialProperty<Real> *> _vel;
  /// Sound speed material properties
  std::vector<const ADMaterialProperty<Real> *> _c;

  /// Time step size
  Real _dt;

public:
  static InputParameters validParams();
};
