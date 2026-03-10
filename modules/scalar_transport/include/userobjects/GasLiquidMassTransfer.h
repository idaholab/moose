//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "Function.h"

// Forward Declarations
class SinglePhaseFluidProperties;

/**
  * Computes the mass transfer coefficient of a gas/liquid interface
  */
class GasLiquidMassTransfer : public GeneralUserObject
{
public:
  static InputParameters validParams();
  GasLiquidMassTransfer(const InputParameters & parameters);
  virtual ~GasLiquidMassTransfer() {}

  virtual void initialSetup();
  virtual void initialize();
  virtual void finalize();
  virtual void execute();
  virtual Real mtc(Real pressure, Real temperature, Real fluid_velocity) const;

  /// Boltzman constant [J/K]
  static constexpr Real _kB = 1.38064852e-23;

protected:


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
