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

class MooseMesh;
class SharpInterfaceRhieChowMassFlux;

class SharpInterfaceFluxBranchConsistencyError : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  SharpInterfaceFluxBranchConsistencyError(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void finalize() override;
  Real getValue() const override;

protected:
  enum class Quantity
  {
    PredictorOperator,
    PressureCorrection,
    Total
  };

  enum class Metric
  {
    L2,
    MaxAbs
  };

  const MooseMesh & _mesh;
  const SharpInterfaceRhieChowMassFlux & _rhie_chow;
  const Quantity _quantity;
  const Metric _metric;
  const Moose::Functor<Real> & _rho;
  const Moose::Functor<Real> * const _vel_x;
  const Moose::Functor<Real> * const _vel_y;
  const Moose::Functor<Real> * const _vel_z;
  Real _value;
};
