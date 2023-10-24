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

class WCNSFVHeatAdvectionPhysics;
class WCNSFVScalarAdvectionPhysics;

/**
 * Creates all the objects needed to add a turbulence model to an incompressible /
 * weakly-compressible Navier Stokes finite volume flow simulation
 */
class WCNSFVTurbulencePhysics : public WCNSFVPhysicsBase
{
public:
  static InputParameters validParams();

  WCNSFVTurbulencePhysics(const InputParameters & parameters);

  bool checkParametersMergeable(const InputParameters & other_params, bool warn) const;

protected:
private:
  void addNonlinearVariables() override;
  void addAuxiliaryVariables() override;
  void addFVKernels() override;
  void addAuxiliaryKernels() override;
  void addMaterials() override;

  /**
   * Functions adding kernels for the turbulence equation(s)
   */
  void addFlowTurbulenceKernels();
  void addFluidEnergyTurbulenceKernels();
  void addScalarAdvectionTurbulenceKernels();

  /// Turbulence model to create the equation(s) for
  const MooseEnum _turbulence_model;

  /// The heat advection physics to add turbulent mixing for
  const WCNSFVHeatAdvectionPhysics * _fluid_energy_physics;
  /// The scalar advection physics to add turbulent mixing for
  const WCNSFVScalarAdvectionPhysics * _scalar_advection_physics;

private:
  /// Name of the mixing length auxiliary variable
  const VariableName _mixing_length_name;
};
