//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WCNSFVPhysicsBase.h"

class WCNSFVTurbulenceModel;

/**
 * Creates all the objects needed to solve the Navier Stokes mass and momentum equations
 */
class WCNSFVFlowPhysics : public WCNSFVPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSFVFlowPhysics(const InputParameters & parameters);

  bool checkParametersMergeable(const InputParameters & other_params, bool warn) const override;

protected:
private:
  void addNonlinearVariables() override;
  void addInitialConditions() override;
  void addFVKernels() override;
  void addFVBCs() override;
  void addMaterials() override;
  void addUserObjects() override;

  /// Function adding kernels for the incompressible continuity equation
  void addINSMassKernels();

  /**
   * Functions adding kernels for the incompressible momentum equation
   * If the material properties are not constant, these can be used for
   * weakly-compressible simulations (except the Boussinesq kernel) as well.
   */
  void addINSMomentumTimeKernels();
  void addINSMomentumViscousDissipationKernels();
  void addINSMomentumMixingLengthKernels();
  void addINSMomentumAdvectionKernels();
  void addINSMomentumPressureKernels();
  void addINSMomentumGravityKernels();
  void addINSMomentumBoussinesqKernels();
  void addINSMomentumFrictionKernels();

  /// Functions which add time kernels for transient, weakly-compressible simulations.
  void addWCNSMassTimeKernels();
  void addWCNSMomentumTimeKernels();

  /// Functions adding boundary conditions for the incompressible simulation.
  /// These are used for weakly-compressible simulations as well.
  void addINSInletBC();
  void addINSOutletBC();
  void addINSWallsBC();

  /// Add material to define the local speed in porous medium flows
  void addPorousMediumSpeedMaterial();

  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  /// The type of the pressure interpolation method
  // const MooseEnum _pressure_face_interpolation;
  // /// The type of the face interpolation method for the velocity/momentum
  // const MooseEnum _momentum_face_interpolation;

  bool hasTurbulencePhysics() const { return !(!_turbulence_physics); }

  const WCNSFVTurbulenceModel * _turbulence_physics;
};
