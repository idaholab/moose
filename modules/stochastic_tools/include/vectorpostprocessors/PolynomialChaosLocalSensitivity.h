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
#include "SamplerInterface.h"
#include "PolynomialChaos.h"
#include "SurrogateModelInterface.h"

class PolynomialChaosLocalSensitivity : public GeneralVectorPostprocessor,
                                        SamplerInterface,
                                        SurrogateModelInterface
{
public:
  static InputParameters validParams();

  PolynomialChaosLocalSensitivity(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// Reference to PolynomialChaos
  const PolynomialChaos & _pc_uo;
  /// Sampler defining points to evaluate
  Sampler * _sampler;
  /// Specific points defined at input
  const std::vector<Real> & _points;
  /// Sensitivity dimension, i.e. parameters to take derivative with respect to
  std::vector<unsigned int> _sdim;
  /// Where or not to output all the points used
  const bool _output_points;
  /// Whether or not we have initialized the vectors
  bool _initialized;
  /// Vectors containing the local sensitivity results
  std::vector<VectorPostprocessorValue *> _sensitivity_vector;
  /// Vector containing all the points for each parameter
  std::vector<VectorPostprocessorValue *> _points_vector;
};
