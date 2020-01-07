//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

/**
 * ChannelGradientBC implements a boundary condition like that for heat transfer
 * between a convecting fluid and a solid surface: \f$q = h(T_{\infty} - T_s)\f$
 * where \f$T_{\infty}\f$ is the bulk fluid temperature, \f$T_s\f$ is the temperature
 * of the solid surface, and \f$h\f$ is a heat transfer coefficient usually obtained
 * through a Nusselt-like correlation. The difference in temperature is provided by
 * a ChannelGradientVectorPostprocessor. Note that this boundary condition is only
 * applicable to boundaries with a single varying coordinate (e.g. constant x and y,
 * varying z)
 */
class ChannelGradientBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  ChannelGradientBC(const InputParameters & parameters);

protected:
  /**
   * This method retrieves the difference in temperature between \f$T_{\infty}\f$
   * and \f$T_s\f$ at the current quadrature point. It does this in a two step process:
   *
   *   1. Binary search to determine which vector postprocessor sampling points the
   *      quadrature point lies between
   *   2. Linear interpolation of the temperature difference between the bounding
   *      sampling points
   */
  virtual Real getGradient();

  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const MooseEnum _axis;
  const VectorPostprocessorValue & _channel_gradient_axis_coordinate;
  const VectorPostprocessorValue & _channel_gradient_value;
  const MaterialProperty<Real> & _h;
};
