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

class MorrisReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  MorrisReporter(const InputParameters & parameters);

  virtual void initialize() override final;

  virtual void execute() override final {}
  virtual void finalize() override final {}
  virtual void store(nlohmann::json & json) const override;

private:
  /**
   * Helper for adding Morris reporter values
   */
  template <typename DataType>
  void declareValueHelper(const ReporterName & r_name);

  /// Morris sampler (don't need any specific functions, but should be this type)
  Sampler & _sampler;

  /// CI levels to be computed
  const std::vector<Real> & _ci_levels;

  /// Number of CI replicates to use in Bootstrap methods
  const unsigned int & _ci_replicates;

  /// Random seed for producing CI replicates
  const unsigned int & _ci_seed;

  /// Whether or not initialize() has been called for reporter value declaration
  bool _initialized;
};

template <typename DataType>
using MorrisState =
    std::map<std::string, std::pair<std::vector<DataType>, std::vector<std::vector<DataType>>>>;

template <typename DataType>
class MorrisReporterContext : public ReporterGeneralContext<MorrisState<DataType>>
{
public:
  MorrisReporterContext(const libMesh::ParallelObject & other,
                        const MooseObject & producer,
                        ReporterState<MorrisState<DataType>> & state,
                        Sampler & sampler,
                        const std::vector<DataType> & data);

  MorrisReporterContext(const libMesh::ParallelObject & other,
                        const MooseObject & producer,
                        ReporterState<MorrisState<DataType>> & state,
                        Sampler & sampler,
                        const std::vector<DataType> & data,
                        const MooseEnum & ci_method,
                        const std::vector<Real> & ci_levels,
                        unsigned int ci_replicates,
                        unsigned int ci_seed);

  virtual void finalize() override;
  virtual std::string type() const override
  {
    return "MorrisSensitivity<" + MooseUtils::prettyCppType<DataType>() + ">";
  }

private:
  /**
   * Function for computing elementary effects for a single set of trajectories
   * This is meant to be specialized for different data types
   */
  std::vector<DataType> computeElementaryEffects(const RealEigenMatrix & x,
                                                 const std::vector<DataType> & y) const;

  /// Morris sampler (don't need any specific functions, but should be this type)
  Sampler & _sampler;

  /// Data used for the statistic calculation
  const std::vector<DataType> & _data;

  /// Storage for the Calculator object for the desired stat, this is created in constructor
  std::unique_ptr<StochasticTools::Calculator<std::vector<DataType>, DataType>> _mu_calc;
  std::unique_ptr<StochasticTools::Calculator<std::vector<DataType>, DataType>> _mustar_calc;
  std::unique_ptr<StochasticTools::Calculator<std::vector<DataType>, DataType>> _sig_calc;

  /// Storage for the BootstrapCalculator for the desired confidence interval calculations (optional)
  std::unique_ptr<StochasticTools::BootstrapCalculator<std::vector<DataType>, DataType>>
      _ci_mu_calc;
  std::unique_ptr<StochasticTools::BootstrapCalculator<std::vector<DataType>, DataType>>
      _ci_mustar_calc;
  std::unique_ptr<StochasticTools::BootstrapCalculator<std::vector<DataType>, DataType>>
      _ci_sig_calc;
};
