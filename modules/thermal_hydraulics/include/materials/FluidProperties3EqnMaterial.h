//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeMaterialInterfaceTHM.h"

class SinglePhaseFluidProperties;

/**
 * Computes velocity and thermodynamic variables from solution variables for 1-phase flow.
 */
class FluidProperties3EqnMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  FluidProperties3EqnMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Cross-sectional area
  const VariableValue & _area;
  const VariableValue & _rhoA;
  const VariableValue & _rhouA;
  const VariableValue & _rhoEA;

  /// Density
  MaterialProperty<Real> & _rho;
  MaterialProperty<Real> & _drho_drhoA;

  /// Specific volume
  MaterialProperty<Real> & _v;
  MaterialProperty<Real> & _dv_drhoA;

  /// Velocity
  MaterialProperty<Real> & _vel;
  MaterialProperty<Real> & _dvel_drhoA;
  MaterialProperty<Real> & _dvel_drhouA;

  /// Specific internal energy
  MaterialProperty<Real> & _e;
  MaterialProperty<Real> & _de_drhoA;
  MaterialProperty<Real> & _de_drhouA;
  MaterialProperty<Real> & _de_drhoEA;

  /// Pressure
  MaterialProperty<Real> & _p;
  MaterialProperty<Real> & _dp_drhoA;
  MaterialProperty<Real> & _dp_drhouA;
  MaterialProperty<Real> & _dp_drhoEA;

  /// Temperature
  MaterialProperty<Real> & _T;
  MaterialProperty<Real> & _dT_drhoA;
  MaterialProperty<Real> & _dT_drhouA;
  MaterialProperty<Real> & _dT_drhoEA;

  /// Specific enthalpy
  MaterialProperty<Real> & _h;
  MaterialProperty<Real> & _dh_drhoA;
  MaterialProperty<Real> & _dh_drhouA;
  MaterialProperty<Real> & _dh_drhoEA;

  /// Specific total (stagnation) enthalpy
  MaterialProperty<Real> & _H;
  MaterialProperty<Real> & _dH_drhoA;
  MaterialProperty<Real> & _dH_drhouA;
  MaterialProperty<Real> & _dH_drhoEA;

  /// Sound speed
  MaterialProperty<Real> & _c;

  /// Constant-pressure specific heat
  MaterialProperty<Real> & _cp;

  /// Constant-volume specific heat
  MaterialProperty<Real> & _cv;

  /// Thermal conductivity
  MaterialProperty<Real> & _k;

  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
