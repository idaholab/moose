//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
#include "PolynomialChaos.h"
#include "SurrogateModelInterface.h"

class PolynomialChaosStatistics : public GeneralVectorPostprocessor, SurrogateModelInterface
{
public:
  static InputParameters validParams();

  PolynomialChaosStatistics(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// The selected statistics to compute
  const MultiMooseEnum & _type;
  /// Vector containing types of statistical values
  VectorPostprocessorValue & _stat_type;
  /// Pointers to PolynomialChaos
  std::vector<const PolynomialChaos *> _pc_uo;
  /// Vectors containing statistical values
  std::vector<VectorPostprocessorValue *> _stats;
};
