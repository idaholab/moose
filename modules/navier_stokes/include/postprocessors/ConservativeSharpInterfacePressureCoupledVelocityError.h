//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class ElemInfo;
class Function;
class MooseMesh;
class ConservativeSharpInterfaceRhieChowMassFlux;

class ConservativeSharpInterfacePressureCoupledVelocityError : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  ConservativeSharpInterfacePressureCoupledVelocityError(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void finalize() override;
  Real getValue() const override;

protected:
  const MooseMesh & _mesh;
  const unsigned int _component;
  const Function & _exact_velocity;
  const ConservativeSharpInterfaceRhieChowMassFlux & _rhie_chow;
  Real _error;
};
