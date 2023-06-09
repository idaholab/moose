//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

/**
 * Constant velocity scalar advection boundary conditions
 */
class NSFVOutflowTemperatureBC : public FVFluxBC
{
public:
  NSFVOutflowTemperatureBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  /// Density
  const Moose::Functor<ADReal> & _adv_quant;

  /// x-velocity
  const Moose::Functor<ADReal> & _u;
  /// y-velocity
  const Moose::Functor<ADReal> * const _v;
  /// z-velocity
  const Moose::Functor<ADReal> * const _w;

  /// Density
  const Moose::Functor<ADReal> & _backflow_T;

  /// the dimension of the simulation
  const unsigned int _dim;
};
