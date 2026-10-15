//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADScalarKernel.h"

class ThermalSolidProperties;

/**
 * Base class for path-integrated incompressible conjugate heat transfer kernels
 */
class ADPipeWallTemperatureScalarKernelBase : public ADScalarKernel
{
  using Base = ADScalarKernel;

public:
  ADPipeWallTemperatureScalarKernelBase(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Temperature of the wall node on the other side of this layer's radial conduction path
  virtual ADReal otherNodeTemperature() const = 0;
  /// Heat flux exchanged with the fluid or ambient environment on this node's interface to the fluid/environment
  virtual ADReal convectiveLinearHeatRate() const = 0;
  /// Derivative of convectiveLinearHeatRate() with respect to this node's own temperature
  virtual Real convectiveJacobian() const = 0;

  /// Solid properties object for the pipe wall
  const ThermalSolidProperties & _sp;
  /// Coupled wall temperature of the upstream node (same radial layer)
  const ADVariableValue & _Tup;
  /// Coupled wall temperature of the downstream node (same radial layer)
  const ADVariableValue & _Tdown;
  /// Property state integration flag
  const bool _is_implicit;
  /// Cross-sectional area of this wall layer
  const Function & _area;
  /// Perimeter of the interface between the inner and outer wall nodes
  const Function & _interface_perimeter;
  /// Radial conduction-path distance between the inner and outer wall nodes
  const Function & _wall_thickness;
  /// Axial length of this node's control volume
  const Function & _length;
  /// Axial distance from this node to the upstream node
  const Function & _upstream_spacing;
  /// Axial distance from this node to the downstream node
  const Function & _downstream_spacing;
  /// Cross-sectional area of the upstream node's wall layer
  const Function & _upstream_area;
  /// Cross-sectional area of the downstream node's wall layer
  const Function & _downstream_area;
};
