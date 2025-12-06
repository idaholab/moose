//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NavierStokesPhysicsBase.h"
#include "WCNSFVCoupledAdvectionPhysicsHelper.h"
#include "NS.h"

class WCNSFVFluidHeatTransferPhysicsBase;
class WCNSFVScalarTransportPhysicsBase;

#define registerWCNSFVTurbulenceBaseTasks(app_name, derived_name)                                  \
  registerNavierStokesPhysicsBaseTasks(app_name, derived_name);                                    \
  registerMooseAction(app_name, derived_name, "get_turbulence_physics");                           \
  registerMooseAction(app_name, derived_name, "add_variables_physics");                            \
  registerMooseAction(app_name, derived_name, "add_ics_physics");                                  \
  registerMooseAction(app_name, derived_name, "add_fv_kernel");                                    \
  registerMooseAction(app_name, derived_name, "add_fv_bc");                                        \
  registerMooseAction(app_name, derived_name, "add_aux_variable");                                 \
  registerMooseAction(app_name, derived_name, "add_aux_kernel");                                   \
  registerMooseAction(app_name, derived_name, "add_materials_physics")

/**
 * Base class for a Physics that creates all the objects needed to add a turbulence model to an
 * incompressible / weakly-compressible Navier Stokes finite volume flow simulation
 */
class WCNSFVTurbulencePhysicsBase : public NavierStokesPhysicsBase,
                                    public WCNSFVCoupledAdvectionPhysicsHelper
{
public:
  static InputParameters validParams();

  WCNSFVTurbulencePhysicsBase(const InputParameters & parameters);

  /// Whether a turbulence model is in use
  bool hasTurbulenceModel() const { return _turbulence_model != "none"; }
  /// The names of the boundaries with turbulence wall functions
  std::vector<BoundaryName> turbulenceWalls() const { return _turbulence_walls; }
  /// The turbulence epsilon wall treatment (same for all turbulence walls currently)
  MooseEnum turbulenceEpsilonWallTreatment() const { return _wall_treatment_eps; }
  /// The turbulence temperature wall treatment (same for all turbulence walls currently)
  MooseEnum turbulenceTemperatureWallTreatment() const { return _wall_treatment_temp; }
  /// The name of the turbulent kinetic energy variable
  MooseFunctorName tkeName() const { return _tke_name; }

protected:
  virtual void actOnAdditionalTasks() override;
  /// Retrieve the other WCNSFVPhysics at play in the simulation to be able
  /// to add the relevant terms (turbulent diffusion notably)
  void retrieveCoupledPhysics();

  virtual void addSolverVariables() override = 0;
  virtual void addAuxiliaryVariables() override;
  virtual void addFVKernels() override = 0;
  virtual void addFVBCs() override = 0;
  virtual void addAuxiliaryKernels() override;
  virtual void addInitialConditions() override;
  virtual void addMaterials() override;

  /// Turbulence model to create the equation(s) for
  const MooseEnum _turbulence_model;

  bool _has_flow_equations;
  bool _has_energy_equation;
  bool _has_scalar_equations;

  /// The heat advection physics to add turbulent mixing for
  const WCNSFVFluidHeatTransferPhysicsBase * _fluid_energy_physics;
  /// The scalar advection physics to add turbulent mixing for
  const WCNSFVScalarTransportPhysicsBase * _scalar_transport_physics;

  /// List of boundaries to act as walls for turbulence models
  std::vector<BoundaryName> _turbulence_walls;
  /// Turbulence wall treatment for epsilon (same for all walls currently)
  MooseEnum _wall_treatment_eps;
  /// Turbulence wall treatment for temperature (same for all walls currently)
  MooseEnum _wall_treatment_temp;
  /// Name of the turbulent kinetic energy
  const VariableName _tke_name;
  /// Name of the turbulent kinetic energy dissipation
  const VariableName _tked_name;
  /// Name of the turbulence viscosity auxiliary variable (or property)
  const VariableName _turbulent_viscosity_name = NS::mu_t;
};
