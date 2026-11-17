//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

class SinglePhaseFluidProperties;

/**
 * Computes a Dittus-Boelter wall heat transfer coefficient functor material property for the
 * lumped path-integrated flow segments.
 */
class ADDittusBoelterHTCFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();
  ADDittusBoelterHTCFunctorMaterial(const InputParameters & parameters);

protected:
  /// Fluid properties of the flowing fluid
  const SinglePhaseFluidProperties & _fp;
  /// Fluid properties of the pure liquid
  const SinglePhaseFluidProperties * _fp_ref;
  /// Mass flow rate through the segment
  const ADVariableValue & _m;
  /// Fluid temperature of the segment
  const ADVariableValue & _T_fluid;
  /// Fluid temperature at the nominal inlet neighbor
  const ADVariableValue & _T_fluid_inlet;
  /// Fluid temperature at the nominal outlet neighbor
  const ADVariableValue & _T_fluid_outlet;
  /// System reference pressure
  const Moose::Functor<ADReal> & _P_ref;
  /// Cross-sectional area of the flow channel
  const Moose::Functor<ADReal> & _flow_area;
  /// Wetted perimeter of the flow channel
  const Moose::Functor<ADReal> & _wetted_perimeter;
};
