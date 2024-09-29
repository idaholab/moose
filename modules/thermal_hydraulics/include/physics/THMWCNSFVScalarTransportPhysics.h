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
#include "WCNSFVScalarTransportPhysics.h"

/**
 * Sets up the 1D scalar transport equations using a finite volume discretization
 */
class THMWCNSFVScalarTransportPhysics : public ThermalHydraulicsFlowPhysics,
                                        public WCNSFVScalarTransportPhysics
{
public:
  static InputParameters validParams();

  THMWCNSFVScalarTransportPhysics(const InputParameters & params);

private:
  virtual void initializePhysicsAdditional() override;
  virtual void actOnAdditionalTasks() override;

  virtual void addNonlinearVariables() override;
  // For nonlinear variables, we use the traditional method of adding initial conditions instead of
  // what is done in Simulation.C, adding ICs with variables
  virtual void addInitialConditions() override;
  virtual void addFVKernels() override;
  virtual void addFVBCs() override;
  virtual void addMaterials() override;

  virtual void addInletBoundaries() override;
  virtual void addOutletBoundaries() override;
  virtual void addFlowJunctions() override;

  virtual void addWallScalarFlux(const std::string & heat_transfer_component,
                                 const ScalarFluxWallEnum & heat_flux_type) override;

  /// Add functor materials that compute the fluxes / pressures on the sides connected to the junction
  void addJunctionFunctorMaterials();

  ///  Add functor materials to perform the necessary calculations for heat fluxes
  void addSurfaceToVolumeConversionFunctor();
  void addScalarTransferFunctorMaterials();
  // void addWallHeatFluxFunctor();
  void addScalarTransferKernels();

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
