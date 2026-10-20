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
#include "ADFunctorInterface.h"

class SinglePhaseFluidProperties;

class ADIncompressibleEnergySPScalarKernel : public ADScalarKernel, public ADFunctorInterface
{
  using Base = ADScalarKernel;

public:
  ADIncompressibleEnergySPScalarKernel(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  /// Fluid properties object
  const SinglePhaseFluidProperties & _fp;
  /// Coupled mass flow rate through the segment
  const ADVariableValue & _m;
  /// Coupled temperature of the upstream segment
  const ADVariableValue & _Tup;
  /// Coupled temperature of the downstream segment
  const ADVariableValue & _Tdown;
  /// Coupled temperature of the component wall
  const ADVariableValue & _Tw;
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
  /// Heat transfer coefficient from functor material
  const Moose::Functor<ADReal> & _h;
};
