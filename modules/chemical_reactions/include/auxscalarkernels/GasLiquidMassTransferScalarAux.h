//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxScalarKernel.h"
#include "Function.h"

// Forward Declarations
class SinglePhaseFluidProperties;

// This AuxKernel performs a calculation of the Henry coefficient for noble gases using the model
// by K. Lee, et al., "Semi-empirical model for Henry's law constant of noble gases in molten salt",
// Scientific Reports (2024) 14:12847, https://doi.org/10.1038/s41598-024-60006-9.

class GasLiquidMassTransferScalarAux : public AuxScalarKernel
{
public:
  static InputParameters validParams();

  GasLiquidMassTransferScalarAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Pressure [Pa]
  const VariableValue & _pressure;

  /// Fluid temperature [K]
  const VariableValue & _temperature;

  /// Liquid velocity [m/s]
  const VariableValue & _fluid_velocity;

  /// Diameter of the flow channel [m]
  const Real _diameter;

  /// fluid properties user object
  const SinglePhaseFluidProperties & _fp;

  /// Enum used to select the type
  const enum class Equationlist { STOKESEINSTEIN, WILKECHANG } _equation_list;

  /// Particle radius [m]
  const Real _radius;

  /// Molecular weight of fluid [kg/mol]
  const Real _mw;

  /// Association parameter for Wilke-Chang model
  const Real _phi;

  /// Constant in the Wilke-Chang model
  const Real _wc;

  /// Dittus-Boelter leading coefficient
  const Real _db;
};
