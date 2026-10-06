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
#include "ScalarKernel.h"
#include "FunctorInterface.h"

class ThermalSolidProperties;

template <bool is_ad>
class PipeWallTemperatureScalarKernelBaseTempl
  : public std::conditional<is_ad, ADScalarKernel, ScalarKernel>::type,
    public FunctorInterface
{
  using Base = typename std::conditional<is_ad, ADScalarKernel, ScalarKernel>::type;

public:
  PipeWallTemperatureScalarKernelBaseTempl(const InputParameters & parameters);
  virtual bool isADObject() const override { return is_ad; };
  static InputParameters validParams();

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Temperature of the wall node on the other side of this layer's radial conduction path
  virtual GenericReal<is_ad> otherNodeTemperature() const = 0;
  /// Heat flux exchanged with the fluid or ambient environment on this node's non-radial side
  virtual GenericReal<is_ad> convectiveResidual() const = 0;
  /// Derivative of convectiveResidual() with respect to this node's own temperature
  virtual Real convectiveJacobian() const = 0;

  /// Solid properties object for the pipe wall
  const ThermalSolidProperties & _sp;
  /// Coupled wall temperature of the upstream node (same radial layer)
  const VariableValue & _Tup;
  /// Coupled wall temperature of the downstream node (same radial layer)
  const VariableValue & _Tdown;
  /// Property state integration flag
  const bool _is_implicit;
  /// Cross-sectional area of this wall layer
  const Moose::Functor<GenericReal<is_ad>> & _area;
  /// Perimeter of the interface between the inner and outer wall nodes
  const Moose::Functor<GenericReal<is_ad>> & _interface_perimeter;
  /// Radial conduction-path distance between the inner and outer wall nodes
  const Moose::Functor<GenericReal<is_ad>> & _interface_thickness;
  /// Axial length of this node's control volume
  const Moose::Functor<GenericReal<is_ad>> & _length;
  /// Axial distance from this node to the upstream node
  const Moose::Functor<GenericReal<is_ad>> & _upstream_spacing;
  /// Axial distance from this node to the downstream node
  const Moose::Functor<GenericReal<is_ad>> & _downstream_spacing;
  /// Cross-sectional area of the upstream node's wall layer
  const Moose::Functor<GenericReal<is_ad>> & _upstream_area;
  /// Cross-sectional area of the downstream node's wall layer
  const Moose::Functor<GenericReal<is_ad>> & _downstream_area;
};
