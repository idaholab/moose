//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowModel.h"

/**
 * Base class for a flow model for a single-phase fluid
 */
class FlowModel1PhaseBase : public FlowModel
{
public:
  static InputParameters validParams();

  FlowModel1PhaseBase(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addInitialConditions() override;
  virtual void addMooseObjects() override;

protected:
  // Methods to get scaling factors for rhoA, rhouA, and rhoEA
  virtual Real getScalingFactorRhoA() const = 0;
  virtual Real getScalingFactorRhoUA() const = 0;
  virtual Real getScalingFactorRhoEA() const = 0;

  /// Returns the solution variable names for the flow model
  virtual std::vector<VariableName> solutionVariableNames() const = 0;

  /// Returns true if all of the IC parameters are valid
  bool ICParametersAreValid() const;
  /// Adds an IC from a function
  void addFunctionIC(const VariableName & var_name, const FunctionName & function_name);
  // various ICs
  virtual void addRhoAIC();
  virtual void addRhoUAIC();
  virtual void addRhoEAIC() = 0;
  virtual void addVelocityIC();
  virtual void addDensityIC() = 0;
  virtual void addSpecificVolumeIC();
  virtual void addSpecificInternalEnergyIC();
  virtual void addSpecificTotalEnthalpyIC();

  /// Adds the kernels
  virtual void addKernels();
  /// Adds a time derivative kernel for the given variable if problem is transient
  void addTimeDerivativeKernelIfTransient(const VariableName & var_name);
  // various kernels
  virtual void addMomentumAreaGradientKernel();
  virtual void addMomentumFrictionKernel();
  virtual void addMomentumGravityKernel();
  virtual void addEnergyGravityKernel();

  /// Adds the DG kernels
  virtual void addDGKernels();

  /// Adds the aux kernels
  virtual void addAuxKernels();
  // various aux kernels
  virtual void addPressureAux() = 0;
  virtual void addTemperatureAux() = 0;
  virtual void addVelocityAux();
  virtual void addDensityAux();
  virtual void addSpecificVolumeAux();
  virtual void addSpecificInternalEnergyAux();
  virtual void addSpecificTotalEnthalpyAux();

  /// Adds materials to compute fluid properties
  virtual void addFluidPropertiesMaterials() = 0;

  /// Adds numerical flux user object
  virtual void addNumericalFluxUserObject() = 0;
  /// Adds RDG objects
  virtual void addRDGMooseObjects();
  /// Adds slope reconstruction material
  virtual void addSlopeReconstructionMaterial() = 0;
  /// Adds DG kernels
  virtual void addRDGAdvectionDGKernels() = 0;

  /// Slope reconstruction type for rDG
  const MooseEnum _rdg_slope_reconstruction;

  /// Numerical flux user object name
  const UserObjectName _numerical_flux_name;
};
