//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowModel1PhaseBase.h"

/**
 * 1D flow model for a binary gas mixture
 */
class FlowModelGasMix : public FlowModel1PhaseBase
{
public:
  static InputParameters validParams();

  FlowModelGasMix(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addInitialConditions() override;

protected:
  virtual Real getScalingFactorRhoA() const override;
  virtual Real getScalingFactorRhoUA() const override;
  virtual Real getScalingFactorRhoEA() const override;

  virtual std::vector<VariableName> solutionVariableNames() const override;

  virtual void addXiRhoAIC();
  virtual void addRhoEAIC() override;
  virtual void addDensityIC() override;

  virtual void addKernels() override;

  virtual void addDGKernels() override;
  virtual void addMassDiffusionSpeciesDGKernel();
  virtual void addMassDiffusionEnergyDGKernel();

  virtual void addAuxKernels() override;
  virtual void addPressureAux() override;
  virtual void addTemperatureAux() override;
  virtual void addMassFractionAux();

  virtual void addFluidPropertiesMaterials() override;

  virtual void addNumericalFluxUserObject() override;
  virtual void addSlopeReconstructionMaterial() override;
  virtual void addRDGAdvectionDGKernels() override;
};
