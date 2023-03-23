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
#include "StochasticReporter.h"
#include "SurrogateModel.h"
#include "ParallelSolutionStorage.h"
#include "MappingInterface.h"

/**
 * A tool for output Sampler data.
 */
class MappingReporter : public StochasticReporter, public MappingInterface
{
public:
  static InputParameters validParams();

  MappingReporter(const InputParameters & parameters);
  virtual void initialize() override {}
  void initialSetup() override;
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  Sampler * _sampler;

  ParallelSolutionStorage * _parallel_storage;

  const UserObjectName _mapping_name;
  const std::vector<VariableName> _variable_names;
  MappingBase * _mapping;

  std::vector<std::vector<std::vector<Real>> *> _vector_real_values;
};
