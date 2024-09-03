//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThermalHydraulicsFlowPhysics.h"

class PhysicsFlowBoundary;

/**
 * Sets up the single-phase Euler flow equations with a RDG discretization
 */
class THMVACESinglePhaseFlowPhysics : public ThermalHydraulicsFlowPhysics
{
public:
  static InputParameters validParams();

  THMVACESinglePhaseFlowPhysics(const InputParameters & params);

protected:
  /// Slope reconstruction type for rDG
  const MooseEnum _rdg_slope_reconstruction;

  /// Numerical flux user object name
  const UserObjectName _numerical_flux_name;

  /// Scaling factors for each solution variable (rhoA, rhouA, rhoEA)
  const std::vector<Real> _scaling_factors;

private:
  virtual void actOnAdditionalTasks() override;

  virtual void addNonlinearVariables() override;
  virtual void addAuxiliaryVariables() override;
  virtual void addTHMInitialConditions();
  virtual void addFEKernels() override;
  virtual void addDGKernels() override;
  virtual void addAuxiliaryKernels() override;
  virtual void addMaterials() override;
  virtual void addUserObjects() override;

  virtual void addInletBoundaries() override;
  virtual void addOutletBoundaries() override;

  /// Adds the boundary flux boundary condition, which uses the boundary flux user object
  void addBoundaryFluxBC(const PhysicsFlowBoundary & comp,
                         const UserObjectName & boundary_numerical_flux_name);

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
