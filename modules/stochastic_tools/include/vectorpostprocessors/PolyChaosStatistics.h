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

class PolyChaosStatistics;

template <>
InputParameters validParams<PolyChaosStatistics>();

class PolyChaosStatistics : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  PolyChaosStatistics(const InputParameters & parameters);

  virtual void initialize() override{};
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Reference to PolynomialChaos
  const PolynomialChaos & _pc_uo;
  /// The selected statistics to compute
  const MultiMooseEnum & _type;
  /// Vector containing types of statistical values
  VectorPostprocessorValue & _stat_type;
  /// Vector containing statistical values
  VectorPostprocessorValue & _stats;
};
