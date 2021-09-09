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

/**
 * ReporterContext that utilizes a Calculator object to compute its value and confidence levels
 */
template <typename InType, typename OutType>
class ReporterStatisticsContext
  : public ReporterGeneralContext<std::pair<OutType, std::vector<OutType>>>
{
public:
  ReporterStatisticsContext(const libMesh::ParallelObject & other,
                            const MooseObject & producer,
                            ReporterState<std::pair<OutType, std::vector<OutType>>> & state,
                            const InType & data,
                            const ReporterProducerEnum & mode,
                            const MooseEnumItem & stat);

  ReporterStatisticsContext(const libMesh::ParallelObject & other,
                            const MooseObject & producer,
                            ReporterState<std::pair<OutType, std::vector<OutType>>> & state,
                            const InType & data,
                            const ReporterProducerEnum & mode,
                            const MooseEnumItem & stat,
                            const MooseEnum & ci_method,
                            const std::vector<Real> & ci_levels,
                            unsigned int ci_replicates,
                            unsigned int ci_seed);

  virtual void finalize() override;
  virtual void storeInfo(nlohmann::json & json) const override;

private:
  /// Data used for the statistic calculation
  const InType & _data;

  /// Mode in which the above data was produced
  const ReporterProducerEnum & _data_mode;

  /// Storage for the Calculator object for the desired stat, this is created in constructor
  std::unique_ptr<StochasticTools::Calculator<InType, OutType>> _calc_ptr;

  /// Storage for the BootstrapCalculator for the desired confidence interval calculations (optional)
  std::unique_ptr<StochasticTools::BootstrapCalculator<InType, OutType>> _ci_calc_ptr = nullptr;
};

template <typename InType, typename OutType>
ReporterStatisticsContext<InType, OutType>::ReporterStatisticsContext(
    const libMesh::ParallelObject & other,
    const MooseObject & producer,
    ReporterState<std::pair<OutType, std::vector<OutType>>> & state,
    const InType & data,
    const ReporterProducerEnum & mode,
    const MooseEnumItem & stat)
  : ReporterGeneralContext<std::pair<OutType, std::vector<OutType>>>(other, producer, state),
    _data(data),
    _data_mode(mode),
    _calc_ptr(StochasticTools::makeCalculator<InType, OutType>(stat, other))
{
}

template <typename InType, typename OutType>
ReporterStatisticsContext<InType, OutType>::ReporterStatisticsContext(
    const libMesh::ParallelObject & other,
    const MooseObject & producer,
    ReporterState<std::pair<OutType, std::vector<OutType>>> & state,
    const InType & data,
    const ReporterProducerEnum & mode,
    const MooseEnumItem & stat,
    const MooseEnum & ci_method,
    const std::vector<Real> & ci_levels,
    unsigned int ci_replicates,
    unsigned int ci_seed)
  : ReporterStatisticsContext<InType, OutType>(other, producer, state, data, mode, stat)
{
  _ci_calc_ptr = StochasticTools::makeBootstrapCalculator<InType, OutType>(
      ci_method, other, ci_levels, ci_replicates, ci_seed, *_calc_ptr);
}

template <typename InType, typename OutType>
void
ReporterStatisticsContext<InType, OutType>::finalize()
{
  if (_data_mode == REPORTER_MODE_DISTRIBUTED || this->processor_id() == 0)
  {
    this->_state.value().first = _calc_ptr->compute(_data, _data_mode == REPORTER_MODE_DISTRIBUTED);

    if (_ci_calc_ptr)
      this->_state.value().second =
          _ci_calc_ptr->compute(_data, _data_mode == REPORTER_MODE_DISTRIBUTED);
  }

  ReporterGeneralContext<std::pair<OutType, std::vector<OutType>>>::finalize();
}

template <typename InType, typename OutType>
void
ReporterStatisticsContext<InType, OutType>::storeInfo(nlohmann::json & json) const
{
  ReporterGeneralContext<std::pair<OutType, std::vector<OutType>>>::storeInfo(json);
  json["stat"] = _calc_ptr->name();
}

/**
 * Compute several metrics for supplied data.
 *
 * This class uses Calculator objects defined in StatisticsReporter.h and is setup such that if a
 * new calculation is needed it can be added in StatisticsReporter.h without modification of this
 * object.
 */
class StatisticsReporter : public GeneralReporter
{
public:
  static InputParameters validParams();
  StatisticsReporter(const InputParameters & parameters);

  /**
   * This is where the reporter values are declared
   * Note: unfortunetly this cannot be in the constructor since the reporter values
   *       containing the data might not exist yet. So we put it here to give the
   *       values their last chance to exist.
   */
  virtual void initialize() final;

  /// Not used; all operations are  wrapped in the ReporterStatisticsContext
  virtual void execute() final{};
  virtual void finalize() final{};
  virtual void store(nlohmann::json & json) const override;

private:
  // Statistics to be computed
  const MultiMooseEnum & _compute_stats;

  // CI Method to be computed (optional)
  const MooseEnum & _ci_method;

  // CI levels to be computed
  const std::vector<Real> & _ci_levels;

  // Number of CI replicates to use in Bootstrap methods
  const unsigned int & _ci_replicates;

  // Random seed for producing CI replicates
  const unsigned int & _ci_seed;

  /// Whether or not initialize() has been called for reporter value declaration
  bool _initialized;

  /**
   * Helper for adding statistic reporter values
   *
   * @param r_name ReporterName of the data from which the statistics will be computed
   */
  template <typename InType, typename OutType>
  void declareValueHelper(const ReporterName & r_name);
};
