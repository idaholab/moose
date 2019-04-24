//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalKernel.h"

class InertialForcePD;
class PeridynamicsMesh;

template <>
InputParameters validParams<InertialForcePD>();

/**
 * Kernel class for inertial term in peridynamic solid mechanics models
 */
class InertialForcePD : public NodalKernel
{
public:
  InertialForcePD(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

private:
  /// Reference to peridynamic mesh object
  PeridynamicsMesh & _pdmesh;

  /// Material density
  const Real _density;
  /// Old displacement value
  const VariableValue & _u_old;

  /// Old velocity value
  const VariableValue & _vel_old;

  /// Old acceleration value
  const VariableValue & _accel_old;

  /// Parameter beta for Newmark time integration scheme
  const Real _beta;
};
