//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowChannelBase.h"

/**
 * Base class for single-phase flow channels
 */
class FlowChannel1PhaseBase : public FlowChannelBase
{
public:
  static InputParameters validParams();

  FlowChannel1PhaseBase(const InputParameters & params);

  virtual void addMooseObjects() override;

  /**
   * Gets 1-phase wall heat transfer coefficient names for connected heat transfers
   */
  std::vector<MaterialPropertyName> getWallHTCNames1Phase() const { return _Hw_1phase_names; }

  /**
   * Gets the numerical flux user object name
   */
  const UserObjectName & getNumericalFluxUserObjectName() const { return _numerical_flux_name; }

  /**
   * Gets the slope reconstruction option used
   */
  const MooseEnum & getSlopeReconstruction() const { return _rdg_slope_reconstruction; }

  /// Returns the names of the IC parameters
  virtual std::vector<std::string> ICParameters() const = 0;

protected:
  virtual void init() override;
  virtual std::shared_ptr<FlowModel> buildFlowModel() override;
  virtual void check() const override;

  /**
   * Adds a material for the hydraulic diameter
   */
  virtual void addHydraulicDiameterMaterial();

  /**
   * Populates heat connection variable names lists
   */
  virtual void getHeatTransferVariableNames() override;

  /// Logs an error if the fluid properties is not valid
  virtual void checkFluidProperties() const = 0;

  /// Returns the flow model class name
  virtual std::string flowModelClassName() const = 0;

  /// 1-phase wall heat transfer coefficient names for connected heat transfers
  std::vector<MaterialPropertyName> _Hw_1phase_names;

  /// Numerical flux user object name
  const UserObjectName _numerical_flux_name;

  /// Slope reconstruction type for rDG
  const MooseEnum _rdg_slope_reconstruction;
};
