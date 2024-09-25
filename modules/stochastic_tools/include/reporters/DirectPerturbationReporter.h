//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "VectorCalculators.h"

#include "nlohmann/json.h"

// Forward declaration
class DirectPerturbationSampler;

class DirectPerturbationReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  DirectPerturbationReporter(const InputParameters & parameters);

  virtual void initialize() override final;

  virtual void execute() override final {}
  virtual void finalize() override final {}

private:
  /**
   * Helper for adding direct perturbation-based reporter values
   */
  template <typename DataType>
  void declareValueHelper(const ReporterName & r_name);

  /// Direct perturbation sampler
  DirectPerturbationSampler & _sampler;

  /// Whether or not initialize() has been called for reporter value declaration
  bool _initialized;
};

template <typename DataType>
class DirectPerturbationReporterContext : public ReporterGeneralContext<std::vector<DataType>>
{
public:
  DirectPerturbationReporterContext(const libMesh::ParallelObject & other,
                                    const MooseObject & producer,
                                    ReporterState<std::vector<DataType>> & state,
                                    DirectPerturbationSampler & sampler,
                                    const std::vector<DataType> & data);

  virtual void finalize() override;
  virtual std::string type() const override
  {
    return "DirectPerturbationSensitivity<" + MooseUtils::prettyCppType<DataType>() + ">";
  }

private:
  /// Compute direct perturbation index, split into a separate function due to
  /// the different operators on vectors and scalars
  void addSensitivityConstribution(DataType & add_to,
                                   const DataType & to_add,
                                   const Real interval) const;

  /// Compute direct perturbation index, split into a separate function due to
  /// the different operators on vectors and scalars
  DataType initializeSensitivity(const DataType & example_output) const;

  /// Reference to the direct perturbation sampler
  DirectPerturbationSampler & _sampler;

  /// Data used for the statistic calculation
  const std::vector<DataType> & _data;
};
