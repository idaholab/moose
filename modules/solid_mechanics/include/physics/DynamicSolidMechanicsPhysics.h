//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "QuasiStaticSolidMechanicsPhysics.h"

class DynamicSolidMechanicsPhysics : public QuasiStaticSolidMechanicsPhysics
{
public:
  static InputParameters validParams();

  DynamicSolidMechanicsPhysics(const InputParameters & params);

  virtual void act() override;

protected:
  virtual std::string getKernelType() override;
  virtual InputParameters getKernelParameters(std::string type) override;

  std::vector<AuxVariableName> _velocities;
  std::vector<AuxVariableName> _accelerations;
};
