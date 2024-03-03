//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeLagrangianStrain.h"

// Calculate the strain for weak plane stress.
class ComputeLagrangianWPSStrain : public ComputeLagrangianStrain
{
public:
  static InputParameters validParams();

  ComputeLagrangianWPSStrain(const InputParameters & params);

protected:
  virtual void computeDeformationGradient() override;

  // For weak plane stress, the user should provide a nonlinear variable as the out-of-plane strain.
  // This nonlinear out-of-plane strain is _solved_ as part of the system to weakly enforce the
  // out-of-plane stress to match a target value.
  const VariableValue & _out_of_plane_strain;
};
