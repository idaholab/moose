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

class ADMHDFormLossFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  ADMHDFormLossFunctorMaterial(const InputParameters & parameters);

protected:
  /// Coupled temperature variable
  const ADVariableValue & _T;
  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;
  /// Coupled mass flow rate through the segment
  const ADVariableValue & _m;
  /// Coupled temperature of the upstream segment
  const VariableValue & _Tup;
  /// Coupled temperature of the downstream segment
  const VariableValue & _Tdown;
  /// Property state integration flag
  const bool _is_implicit;
  /// System reference pressure
  const Moose::Functor<ADReal> & _Pref;
  /// Flow area of the segment
  const Moose::Functor<ADReal> & _area;
  /// Wetted perimeter of the segment
  const Moose::Functor<ADReal> & _perimeter;
  /// Length of the segment
  const Moose::Functor<ADReal> & _length;
  /// Fluid conductivity
  const Moose::Functor<ADReal> & _sigf;
  /// Wall conductivity
  const Moose::Functor<ADReal> & _sigw;
  /// Wall thickness
  const Moose::Functor<ADReal> & _tw;
  /// Magnetic field strength
  const Moose::Functor<ADReal> & _B;
};
