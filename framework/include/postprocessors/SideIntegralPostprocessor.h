//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SidePostprocessor.h"

/**
 * This postprocessor computes a surface integral of the specified variable on
 * a sideset on the boundary of the mesh.
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class SideIntegralPostprocessor : public SidePostprocessor
{
public:
  static InputParameters validParams();

  SideIntegralPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual Real computeQpIntegral() = 0;
  virtual Real computeFaceInfoIntegral(const FaceInfo * /* fi */)
  {
    mooseError("Integral over faces have not been implemented for this postprocessor");
  };
  virtual Real computeIntegral();

  /// The local quadrature point index when computing an integral over quadrature points
  unsigned int _qp;

  /// Holds the postprocessor result, the integral
  Real _integral_value;

  /// Whether to integrate over quadrature points or FaceInfos
  bool _qp_integration;
};
