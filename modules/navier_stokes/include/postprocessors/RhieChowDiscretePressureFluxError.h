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
class RhieChowMassFlux;

class RhieChowDiscretePressureFluxError : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  RhieChowDiscretePressureFluxError(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void finalize() override;
  Real getValue() const override;

protected:
  enum class Metric
  {
    L2,
    MaxAbs
  };

  const MooseMesh & _mesh;
  const RhieChowMassFlux & _rhie_chow;
  const Function & _exact_pressure;
  const Metric _metric;
  Real _value;
};
