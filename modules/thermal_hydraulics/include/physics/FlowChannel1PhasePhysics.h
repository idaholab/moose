//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PhysicsBase.h"
#include "GravityInterface.h"
#include "NamingInterface.h"
#include "THMVariableCoordinator.h"
#include "FlowChannelClosuresInterface.h"

/**
 * Single-phase flow (mass, momentum, energy) physics for a 1D flow channel ActionComponent (see
 * FlowChannel1PhaseAC), which creates and attaches one of these to itself in its constructor.
 *
 * This implements FlowChannelClosuresInterface so that the same ClosuresBase-derived objects
 * (Closures1PhaseSimple, Closures1PhaseTHM, ...) that classic THM's FlowChannelBase uses for wall
 * friction (and, in the future, wall heat transfer) also work here - see
 * FlowChannelClosuresInterface for why. Wall heat transfer is not yet wired in: this Physics has no
 * heat-transfer-connection bookkeeping (getClosuresNumberOfHeatTransferConnections() always
 * returns 0), since there is no native heat-transfer-to-flow-channel ActionComponent yet; wall-HTC
 * materials remain in classic THM's FlowChannelBase/HeatTransferBase Component path until one
 * exists.
 *
 * This class is currently only meant to be attached to a single flow channel component; using it
 * on more than one component would incorrectly apply one component's fluid properties/area/initial
 * conditions to all of them.
 */
class FlowChannel1PhasePhysics : public PhysicsBase,
                                 public GravityInterface,
                                 public NamingInterface,
                                 public FlowChannelClosuresInterface
{
public:
  static InputParameters validParams();
  FlowChannel1PhasePhysics(const InputParameters & params);

  /// Name of the user object that defines fluid properties for this flow channel
  const UserObjectName & fluidPropertiesName() const { return _fp_name; }
  /// Name of the numerical flux user object this Physics creates, used by boundary conditions
  const UserObjectName & numericalFluxUserObjectName() const { return _numerical_flux_name; }

  // FlowChannelClosuresInterface implementation ----
  virtual const std::vector<SubdomainName> & getClosuresBlocks() const override { return _blocks; }
  virtual const std::string & getClosuresName() const override { return name(); }
  virtual bool getClosuresTemperatureMode() const override { return false; }
  virtual unsigned int getClosuresNumberOfHeatTransferConnections() const override { return 0; }
  virtual std::vector<VariableName> getClosuresHeatedPerimeterNames() const override { return {}; }
  virtual std::vector<VariableName> getClosuresWallTemperatureNames() const override { return {}; }
  virtual HeatTransferGeometry getClosuresHeatTransferGeometry() const override
  {
    return HeatTransferGeometry::PIPE;
  }
  virtual PipeLocation getClosuresPipeLocation() const override { return PipeLocation::INTERIOR; }
  virtual bool hasClosuresWallFrictionFactorFunction() const override { return isParamValid("f"); }
  virtual const FunctionName & getClosuresWallFrictionFactorFunction() const override
  {
    return getParam<FunctionName>("f");
  }
  virtual Real getClosuresRoughness() const override { return getParam<Real>("roughness"); }
  virtual Real getClosuresPoD() const override { return getParam<Real>("PoD"); }
  virtual void connectClosuresObject(const InputParameters & obj_params,
                                     const std::string & obj_name,
                                     const std::string & param) const override;

protected:
  virtual void addSolverVariables() override;
  virtual void addAuxiliaryVariables() override;
  virtual void addInitialConditions() override;
  virtual void addFEKernels() override;
  virtual void addDGKernels() override;
  virtual void addMaterials() override;
  virtual void addUserObjects() override;
  virtual void addAuxiliaryKernels() override;
  virtual void checkIntegrity() const override;

  virtual std::vector<UserObjectName> getSuppliedUserObjects() const override
  {
    return {_numerical_flux_name};
  }

private:
  /// Adds the hydraulic diameter material, needed by the wall friction kernel/closures
  /// correlations - mirrors FlowChannel1PhaseBase::addHydraulicDiameterMaterial()
  void addHydraulicDiameterMaterial();
  /// Adds a nonlinear (nl = true) or auxiliary (nl = false) flow variable on this Physics' blocks
  void addFlowVariable(bool nl,
                       const VariableName & var_name,
                       const std::string & family = "MONOMIAL",
                       const std::string & order = "CONSTANT",
                       Real scaling_factor = 1.0);
  /// Adds a FunctionIC for a variable
  void addFunctionIC(const VariableName & var_name, const FunctionName & function_name);
  /// Seeds AREA/AREA_LINEAR's initial value so other ICs (e.g. rhoA = density * area) can read it
  void addAreaInitialCondition();
  /// Adds an ADTimeDerivative kernel for the given variable if the physics is transient
  void addTimeDerivativeKernelIfTransient(const VariableName & var_name);

  /// Name of the user object that defines fluid properties
  const UserObjectName _fp_name;
  /// Area of the flow channel; can be a constant or a function
  const FunctionName _area_fn_name;
  /// Initial pressure, temperature and velocity functions
  const FunctionName _initial_p_fn;
  const FunctionName _initial_T_fn;
  const FunctionName _initial_vel_fn;
  /// Scaling factors for rhoA, rhouA, rhoEA
  const std::vector<Real> _scaling_factors;
  /// Name of the numerical flux user object this Physics creates
  const UserObjectName _numerical_flux_name;
  /// Coordinator that merges this and other components' requests for shared, bare-named
  /// variables (rhoA, A, ...) into single MOOSE variables spanning the union of their blocks
  THMVariableCoordinator & _coordinator;
};
