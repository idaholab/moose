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

class RhieChowMassFlux;

class RhieChowFaceFluxConsistencyError : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  RhieChowFaceFluxConsistencyError(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void finalize() override;
  Real getValue() const override;

protected:
  enum class Quantity
  {
    TotalL2,
    InternalL2,
    BoundaryL2
  };

  const RhieChowMassFlux & _rhie_chow;
  const Quantity _quantity;
};
