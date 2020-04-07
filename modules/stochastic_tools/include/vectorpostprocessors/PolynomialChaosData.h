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

class PolynomialChaosData : public GeneralVectorPostprocessor, public SurrogateModelInterface
{
public:
  static InputParameters validParams();

  PolynomialChaosData(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override{};

protected:
  /// Reference to PolynomialChaos
  const PolynomialChaos & _pc_uo;

  /// Reference to PC user object coefficient vector
  const std::vector<Real> & _coeff;
  /// Vector containing PCE coefficients
  VectorPostprocessorValue & _coeff_vector;
  /// Matrix of 1-D polynomial orders
  std::vector<VectorPostprocessorValue *> _order_vector;
  /// Whether we have been through initialize() yet
  bool _initialized;
};
