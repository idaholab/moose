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
 * Temperature advection boundary condition allowing for inflow and outflow
 */
class NSFVOutflowTemperatureBC : public FVFluxBC
{
public:
  NSFVOutflowTemperatureBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  /// Density
  const Moose::Functor<ADReal> & _rho;

  /// Specific Heat at Constant Pressure
  const Moose::Functor<ADReal> & _cp;

  /// x-velocity
  const Moose::Functor<ADReal> & _u;
  /// y-velocity
  const Moose::Functor<ADReal> * const _v;
  /// z-velocity
  const Moose::Functor<ADReal> * const _w;

  /// Backflow Temperature
  const Moose::Functor<ADReal> & _backflow_T;

  /// the dimension of the simulation
  const unsigned int _dim;
};
