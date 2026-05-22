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

class Function;
class MooseMesh;

class ConservativeSharpInterfaceVolumetricFluxConsistencyError : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  ConservativeSharpInterfaceVolumetricFluxConsistencyError(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void finalize() override;
  Real getValue() const override;

protected:
  const MooseMesh & _mesh;
  const Moose::Functor<Real> & _final_phi;
  const Moose::Functor<Real> & _vel_x;
  const Moose::Functor<Real> * const _vel_y;
  const Moose::Functor<Real> * const _vel_z;
  Real _error;
};
