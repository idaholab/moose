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

/**
 * Reporter class for computing and displaying local sensitivity
 * coefficients around a nominal value using a direct perturbation
 * method.
 */
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

  /// If relative sensitivity should be computed
  const bool _relative_sensitivity;

  /// Whether or not initialize() has been called for reporter value declaration
  bool _initialized;
};

/**
 * Reporter context for computing direct perturbation-based sensitivity
 * coefficients
 */
template <typename DataType>
class DirectPerturbationReporterContext : public ReporterGeneralContext<std::vector<DataType>>
{
public:
  /**
   * Constructor
   * @param other A parallel object, usually the MooseApp
   * @param producer The producer object for the reporter
   * @param state A reporter state (a vector of some types in this case)
   * @param sampler The sampler holding information on the direct perturbation paraemters
   * @param data The data coming back from the executed models
   */
  DirectPerturbationReporterContext(const libMesh::ParallelObject & other,
                                    const MooseObject & producer,
                                    ReporterState<std::vector<DataType>> & state,
                                    DirectPerturbationSampler & sampler,
                                    const std::vector<DataType> & data,
                                    const bool relative_sensitivity);

  virtual void finalize() override;
  virtual std::string type() const override
  {
    return "DirectPerturbationSensitivity<" + MooseUtils::prettyCppType<DataType>() + ">";
  }

private:
  /// Compute direct perturbation sensitivity, split into a separate function due to
  /// the different operators on vectors and scalars
  /// @param add_to The data structure which will be extended
  /// @param to_add The data structure which will be added to the other one
  /// @param reference_value The reference values in case we are computing relative sensitivities
  /// @param interval The interval scaling coefficient vector
  void addSensitivityContribution(DataType & add_to,
                                  const DataType & to_add,
                                  const DataType & reference_value,
                                  const Real interval) const;

  /// Initialize the sensitivity container, split into a separate function due to
  /// the different constructors for scalars and vectors
  /// @param example_output A structure which supplies the dimensions for the allocation
  DataType initializeDataType(const DataType & example_output) const;

  /// Reference to the direct perturbation sampler
  DirectPerturbationSampler & _sampler;

  /// Data used for the statistic calculation
  const std::vector<DataType> & _data;

  /// If relative sensitivities should be computed
  const bool _relative_sensitivity;
};
