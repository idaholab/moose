//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RayKernelBase.h"

// Forward declarations
class Assembly;
class MooseMesh;

/**
 * Base class for a RayKernel that integrates along a Ray segment
 */
class IntegralRayKernelBase : public RayKernelBase
{
public:
  IntegralRayKernelBase(const InputParameters & params);

  static InputParameters validParams();

  virtual void preExecuteStudy() override;
  virtual void onSegment() override = 0;

protected:
  /// Reference to the Assembly object
  Assembly & _assembly;

  /// The physical location of the segment's quadrature points, indexed by _qp
  const MooseArray<Point> & _q_point;

  /// The current quadrature point weight value
  const MooseArray<Real> & _JxW;

  /// The current quadrature point index
  unsigned int _qp;
};
