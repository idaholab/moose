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

class DisplacedMeshBlockVolume : public GeneralPostprocessor
{
public:
  static InputParameters validParams();
  DisplacedMeshBlockVolume(const InputParameters & parameters);
  virtual void meshDisplaced() override;
  virtual void execute() override {};
  virtual void initialize() override {}
  virtual Real getValue() const override { return _volume; }

protected:
  MooseMesh & _mesh;
  Real _volume;
};
