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

class WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics;

/**
 * Creates the energy equation of a two-phase diffuse-interface flow, solved for the conserved
 * mixture enthalpy density rho*h = sum_k alpha_k rho_k h_k(T) with the temperature recovered from
 * it after each energy solve. Each phase is advected with the volume-fraction and mass fluxes of
 * the coupled volume-fraction physics, so that rho*h stays consistent with the volume fraction.
 */
class WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics final
  : public NavierStokesPhysicsBase,
    public WCNSFVCoupledAdvectionPhysicsHelper
{
public:
  static InputParameters validParams();

  WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics(const InputParameters & parameters);

private:
  void addSolverVariables() override;
  void addAuxiliaryVariables() override;
  void addInitialConditions() override;
  /// Adds the Boussinesq buoyancy density
  void addMaterials() override;
  void addUserObjects() override;
  void addFVKernels() override;
  void addFVBCs() override;

  /// The kernels read only the two cells of a face, which the flow already ghosts
  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override;

  /// Adds a rho*h boundary condition with either a temperature or a heat flux
  void addEnthalpyBC(const BoundaryName & boundary,
                     const std::string & parameter,
                     const MooseFunctorName & functor);

  /// The volume-fraction physics providing the volume fraction, phase densities and fluxes
  const WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics * const _vof_physics;
  const VariableName _enthalpy_name;
  const VariableName _temperature_name;
  /// Name of the volume-fraction variable of the coupled physics
  const VariableName _alpha_name;
  /// Name of the user object relating rho*h and T
  const UserObjectName _mixture_enthalpy_name;
};
