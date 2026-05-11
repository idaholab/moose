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

class WCNSLinearFVSharpInterfaceVOFPhysics final : public NavierStokesPhysicsBase,
                                                   public WCNSFVCoupledAdvectionPhysicsHelper
{
public:
  static InputParameters validParams();

  WCNSLinearFVSharpInterfaceVOFPhysics(const InputParameters & parameters);

protected:
  void addSolverVariables() override;
  void addInitialConditions() override;
  void addFVKernels() override;
  void addFVBCs() override;
  void addMaterials() override;
  void addUserObjects() override;

  unsigned short getNumberAlgebraicGhostingLayersNeeded() const override { return 2; }

private:
  void addAlphaTimeKernels();
  void addAlphaAdvectionKernels();
  void addAlphaCompressionKernels();
  void addAlphaInletBC();
  void addAlphaOutletBC();

  const VariableName _alpha_name;
  const MooseFunctorName _gas_fraction_name;
  const std::vector<MooseFunctorName> _alpha_inlet_functors;
  const MooseEnum _alpha_outlet_type;
  const MooseFunctorName _alpha_outlet_backflow_functor;
  const bool _alpha_two_term_bc_expansion;
  const MooseFunctorName _compression_factor_name;
  const MooseFunctorName _interface_normal_functor_name;
  const MooseEnum _alpha_correction_scheme;
  const bool _use_mules_correction;

  const bool _create_complementary_fraction;
  const bool _create_mixture_materials;
  const MooseFunctorName _mixture_density_name;
  const MooseFunctorName _mixture_dynamic_viscosity_name;
  const MooseFunctorName _liquid_density_name;
  const MooseFunctorName _gas_density_name;
  const MooseFunctorName _liquid_dynamic_viscosity_name;
  const MooseFunctorName _gas_dynamic_viscosity_name;
};
