//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "GeneralUserObject.h"

class RhieChowMassFlux;

/// Exercises invalid Rhie-Chow sequencing for assertion tests.
class TestRhieChowMassFluxSequence : public GeneralUserObject
{
public:
  static InputParameters validParams();

  TestRhieChowMassFluxSequence(const InputParameters & parameters);

  void initialize() override {}
  void execute() override;
  void finalize() override {}

private:
  RhieChowMassFlux & _rhie_chow;
  const MooseEnum _operation;
};
