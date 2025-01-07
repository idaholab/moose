//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NavierStokesPhysicsBase.h"
#include "WCNSFVCoupledAdvectionPhysicsHelper.h"

#define registerWCNSFVScalarTransportBaseTasks(app_name, derived_name)                             \
  registerMooseAction(app_name, derived_name, "add_variable");                                     \
  registerMooseAction(app_name, derived_name, "add_ic");                                           \
  registerMooseAction(app_name, derived_name, "add_fv_kernel");                                    \
  registerMooseAction(app_name, derived_name, "add_fv_bc")

/**
 * Creates all the objects needed to solve the Navier Stokes scalar transport equations
 */
class WCNSFVScalarTransportPhysicsBase : public NavierStokesPhysicsBase,
                                         public WCNSFVCoupledAdvectionPhysicsHelper
{
public:
  static InputParameters validParams();

  WCNSFVScalarTransportPhysicsBase(const InputParameters & parameters);

  /// Get the names of the advected scalar quantity variables
  const std::vector<NonlinearVariableName> & getAdvectedScalarNames() const
  {
    return _passive_scalar_names;
  }

  /// Whether the physics is actually creating the scalar advection equations
  bool hasScalarEquations() const { return _has_scalar_equation; }

  /**
   * Add a boundary condition on the specified boundary
   * @param boundary_name the name of the boundary
   * @param inlet_type the type of the inlet
   * @param inlet_functor the functor setting the boundary condition
   */
  void addInletBoundary(const BoundaryName & boundary_name,
                        const MooseEnum & inlet_type,
                        const MooseFunctorName & inlet_functor,
                        const unsigned int scalar_index);

  /** Add scalar sources on the specified blocks for all the scalars at once
   * @param blocks subdomains to add the sources on
   * @param scalar_sources vector of the sources, for each passive scalar
   * @param scalar_sources_coefs vector of the functor coefficients multiplying each source
   */
  void addExternalScalarSources(std::vector<SubdomainName> blocks,
                                std::vector<MooseFunctorName> scalar_sources,
                                std::vector<MooseFunctorName> scalar_sources_coefs);

protected:
  virtual void addFVKernels() override;
  virtual void addFVBCs() override;
  virtual void setSlipVelocityParams(InputParameters & /* params */) const {}

  /// Names of the passive scalar variables
  std::vector<NonlinearVariableName> _passive_scalar_names;
  /// A boolean to help compatibility with the old Modules/NavierStokesFV syntax
  /// or to deliberately skip adding the equations (for example for mixtures with a stationary phase)
  const bool _has_scalar_equation;

  /// Passive scalar inlet boundary types
  std::vector<std::map<BoundaryName, MooseEnum>> _passive_scalar_inlet_types;
  /// Functors describing the inlet boundary values. See passive_scalar_inlet_types for what the functors actually represent
  std::vector<std::map<BoundaryName, MooseFunctorName>> _passive_scalar_inlet_functors;

  /// Functors for the passive scalar sources. Indexing is scalar variable index
  std::vector<MooseFunctorName> _passive_scalar_sources;
  /// Subdomains for the passive scalar coupled sources
  std::vector<std::vector<SubdomainName>> _passive_scalar_coupled_sources_blocks;
  /// Functors for the passive scalar (coupled) sources. Inner indexing is scalar variable index
  std::vector<std::vector<MooseFunctorName>> _passive_scalar_coupled_sources;
  /// Coefficients multiplying for the passive scalar sources. Inner indexing is scalar variable index
  std::vector<std::vector<MooseFunctorName>> _passive_scalar_coupled_sources_coefs;

  virtual void addInitialConditions() override;

private:
  /**
   * Functions adding kernels for the incompressible / weakly-compressible scalar transport
   * equation
   * If the material properties are not constant, some of these can be used for
   * weakly-compressible simulations as well.
   */
  virtual void addScalarTimeKernels() = 0;
  virtual void addScalarDiffusionKernels() = 0;
  virtual void addScalarAdvectionKernels() = 0;
  /// Equivalent of NSFVAction addScalarCoupledSourceKernels
  virtual void addScalarSourceKernels() = 0;

  /// Functions adding boundary conditions for the incompressible simulation.
  /// These are used for weakly-compressible simulations as well.
  virtual void addScalarInletBC() = 0;
  virtual void addScalarWallBC() = 0;
  virtual void addScalarOutletBC() = 0;

  virtual unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;
};
