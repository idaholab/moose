//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowModel1PhaseBase.h"

/**
 * Flow model for a single-component, single-phase fluid using the Euler equations
 */
class FlowModelSinglePhase : public FlowModel1PhaseBase
{
public:
  static InputParameters validParams();

  FlowModelSinglePhase(const InputParameters & params);

protected:
  virtual Real getScalingFactorRhoA() const override;
  virtual Real getScalingFactorRhoUA() const override;
  virtual Real getScalingFactorRhoEA() const override;

  virtual std::vector<VariableName> solutionVariableNames() const override;

  virtual void addRhoEAIC() override;
  virtual void addDensityIC() override;

  virtual void addPressureAux() override;
  virtual void addTemperatureAux() override;

  virtual void addFluidPropertiesMaterials() override;

  virtual void addNumericalFluxUserObject() override;
  virtual void addSlopeReconstructionMaterial() override;
  virtual void addRDGAdvectionDGKernels() override;

  /// Scaling factors for each solution variable (rhoA, rhouA, rhoEA)
  const std::vector<Real> _scaling_factors;

public:
  static const std::string DENSITY;
  static const std::string FRICTION_FACTOR_DARCY;
  static const std::string DYNAMIC_VISCOSITY;
  static const std::string HEAT_TRANSFER_COEFFICIENT_WALL;
  static const std::string HYDRAULIC_DIAMETER;
  static const std::string PRESSURE;
  static const std::string RHOA;
  static const std::string RHOEA;
  static const std::string RHOUA;
  static const std::string SOUND_SPEED;
  static const std::string SPECIFIC_HEAT_CONSTANT_PRESSURE;
  static const std::string SPECIFIC_HEAT_CONSTANT_VOLUME;
  static const std::string SPECIFIC_INTERNAL_ENERGY;
  static const std::string SPECIFIC_TOTAL_ENTHALPY;
  static const std::string SPECIFIC_VOLUME;
  static const std::string TEMPERATURE;
  static const std::string THERMAL_CONDUCTIVITY;
  static const std::string VELOCITY;
  static const std::string VELOCITY_X;
  static const std::string VELOCITY_Y;
  static const std::string VELOCITY_Z;
  static const std::string REYNOLDS_NUMBER;
};
