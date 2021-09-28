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
#include "SobolCalculators.h"
#include "VectorOfVectorCalculators.h"

class SobolSampler;
template <typename InType, typename OutType>
class SobolReporterContext;

/**
 * Computes Sobol sensitivity indices, see SobolCalculators
 */
class SobolReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  SobolReporter(const InputParameters & parameters);

  virtual void initialSetup() override {}
  virtual void initialize() override;
  virtual void execute() override {}
  virtual void finalize() override {}

  virtual void store(nlohmann::json & json) const override;

private:
  /**
   * Helper for adding Sobol reporter values
   *
   * @param r_name ReporterName of the data from which the statistics will be computed
   */
  template <typename InType>
  void declareValueHelper(const ReporterName & r_name);

  /// The sampler that generated the samples that produced results for the _results_vectors
  const SobolSampler & _sobol_sampler;

  /// CI levels to be computed
  const std::vector<Real> & _ci_levels;

  /// Number of CI replicates to use in Bootstrap methods
  const unsigned int & _ci_replicates;

  /// Random seed for producing CI replicates
  const unsigned int & _ci_seed;

  /// Whether or not initialize() has been called for reporter value declaration
  bool _initialized;
};

template <typename OutType>
using SobolState = std::pair<std::vector<OutType>, std::vector<std::vector<OutType>>>;

template <typename InType, typename OutType>
class SobolReporterContext : public ReporterGeneralContext<SobolState<OutType>>
{
public:
  SobolReporterContext(const libMesh::ParallelObject & other,
                       const MooseObject & producer,
                       ReporterState<SobolState<OutType>> & state,
                       const InType & data,
                       const ReporterProducerEnum & mode,
                       const SobolSampler & sampler);

  SobolReporterContext(const libMesh::ParallelObject & other,
                       const MooseObject & producer,
                       ReporterState<SobolState<OutType>> & state,
                       const InType & data,
                       const ReporterProducerEnum & mode,
                       const SobolSampler & sampler,
                       const MooseEnum & ci_method,
                       const std::vector<Real> & ci_levels,
                       unsigned int ci_replicates,
                       unsigned int ci_seed);

  virtual void finalize() override;
  virtual std::string type() const override
  {
    return "SobolIndices<" + MooseUtils::prettyCppType<OutType>() + ">";
  }
  static void
  storeSobol(nlohmann::json & json, const SobolState<OutType> & val, unsigned int nparam);

protected:
  virtual void store(nlohmann::json & json) const override;

private:
  /// Data used for the statistic calculation
  const InType & _data;

  /// Mode in which the above data was produced
  const ReporterProducerEnum & _data_mode;

  /// Sobol sampler to get info on number of matrices and whatnot
  const SobolSampler & _sampler;

  /// Storage for the SobolCalculator object, this is created in constructor
  StochasticTools::SobolCalculator<InType, OutType> _calc;

  /// Storage for the BootstrapCalculator for the desired confidence interval calculations (optional)
  std::unique_ptr<StochasticTools::BootstrapCalculator<std::vector<InType>, std::vector<OutType>>>
      _ci_calc_ptr = nullptr;
};
