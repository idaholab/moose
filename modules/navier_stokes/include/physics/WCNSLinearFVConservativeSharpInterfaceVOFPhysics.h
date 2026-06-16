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

class WCNSLinearFVConservativeSharpInterfaceVOFPhysics final
  : public NavierStokesPhysicsBase,
    public WCNSFVCoupledAdvectionPhysicsHelper
{
public:
  static InputParameters validParams();

  WCNSLinearFVConservativeSharpInterfaceVOFPhysics(const InputParameters & parameters);

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
  void addAlphaInletBC();
  void addAlphaOutletBC();
  void addMixtureFunctorMaterial(const std::string & object_suffix,
                                 const MooseFunctorName & property_name,
                                 const MooseFunctorName & phase_1_name,
                                 const MooseFunctorName & phase_2_name,
                                 bool limit_phase_fraction);

  const VariableName _alpha_name;
};
