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
#include "Calculators.h"
#include "BootstrapCalculators.h"

#include "nlohmann/json.h"

/**
 * ReporterContext that utilizes a Calculator object to compute its value and confidence levels
 */
template <typename InType, typename OutType>
class ReporterStatisticsContext : public ReporterContext<OutType>
{
public:
  ReporterStatisticsContext(const libMesh::ParallelObject & other,
                            ReporterState<OutType> & state,
                            const InType & data,
                            const ReporterProducerEnum & mode,
                            const MooseEnumItem & stat);

  ReporterStatisticsContext(const libMesh::ParallelObject & other,
                            ReporterState<OutType> & state,
                            const InType & data,
                            const ReporterProducerEnum & mode,
                            const MooseEnumItem & stat,
                            const MooseEnum & ci_method,
                            const std::vector<Real> & ci_levels,
                            unsigned int ci_replicates,
                            unsigned int ci_seed);

  virtual void finalize() override;
  virtual void store(nlohmann::json & json) const override;

private:
  /// Data used for the statistic calculation
  const InType & _data;

  /// Mode in which the above data was produced
  const ReporterProducerEnum & _data_mode;

  /// Storage for the Calculator object for the desired stat, this is created in constructor
  std::unique_ptr<const StochasticTools::Calculator<InType, OutType>> _calc_ptr;

  /// Storage for the BootstrapCalculator for the desired confidence interval calculations (optional)
  std::unique_ptr<const StochasticTools::BootstrapCalculator<InType, OutType>> _ci_calc_ptr =
      nullptr;

  /// The results
  std::vector<OutType> _ci_results;
};

template <typename InType, typename OutType>
ReporterStatisticsContext<InType, OutType>::ReporterStatisticsContext(
    const libMesh::ParallelObject & other,
    ReporterState<OutType> & state,
    const InType & data,
    const ReporterProducerEnum & mode,
    const MooseEnumItem & stat)
  : ReporterContext<OutType>(other, state),
    _data(data),
    _data_mode(mode),
    _calc_ptr(StochasticTools::makeCalculator<InType, OutType>(stat, other))
{
}

template <typename InType, typename OutType>
ReporterStatisticsContext<InType, OutType>::ReporterStatisticsContext(
    const libMesh::ParallelObject & other,
    ReporterState<OutType> & state,
    const InType & data,
    const ReporterProducerEnum & mode,
    const MooseEnumItem & stat,
    const MooseEnum & ci_method,
    const std::vector<Real> & ci_levels,
    unsigned int ci_replicates,
    unsigned int ci_seed)
  : ReporterStatisticsContext<InType, OutType>(other, state, data, mode, stat)
{
  _ci_calc_ptr = StochasticTools::makeBootstrapCalculator<InType, OutType>(
      ci_method, other, ci_levels, ci_replicates, ci_seed, *_calc_ptr);
}

template <typename InType, typename OutType>
void
ReporterStatisticsContext<InType, OutType>::finalize()
{
  this->_state.value() = _calc_ptr->compute(_data, _data_mode == REPORTER_MODE_DISTRIBUTED);
  ReporterContext<OutType>::finalize();

  if (_ci_calc_ptr)
    _ci_results = _ci_calc_ptr->compute(_data, _data_mode == REPORTER_MODE_DISTRIBUTED);
}

template <typename InType, typename OutType>
void
ReporterStatisticsContext<InType, OutType>::store(nlohmann::json & json) const
{
  ReporterContext<OutType>::store(json);
  json["stat"] = _calc_ptr->name();
  if (_ci_calc_ptr)
    json["confidence_intervals"] = {{"method", _ci_calc_ptr->name()},
                                    {"values", _ci_results},
                                    {"levels", _ci_calc_ptr->levels()},
                                    {"replicates", _ci_calc_ptr->replicates()},
                                    {"seed", _ci_calc_ptr->seed()}};
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

  /// Not used; all operations are  wrapped in the ReporterStatisticsContext
  virtual void execute() final{};
  virtual void initialize() final{};
  virtual void finalize() final{};

private:
  // Statistics to be computed
  const MultiMooseEnum & _compute_stats;

  // CI Method to be computed (optional)
  const MooseEnum & _ci_method;

  // CI levels to be computed (not a ref. by design since these are computed from input parameter)
  const std::vector<Real> _ci_levels;

  // Number of CI replicates to use in Bootstrap methods
  const unsigned int & _ci_replicates;

  // Random seed for producing CI replicates
  const unsigned int & _ci_seed;

  /**
   * Helper function for converting confidence levels given in (0, 0.5] into levels in (0, 1).
   * For example, levels_in = {0.05, 0.1, 0.5} converts to {0.05 0.1, 0.5, 0.9, 0.95}.
   *
   * This also performs error checking on the supplied "ci_levels".
   */
  std::vector<Real> computeLevels(const std::vector<Real> & levels_in) const;

  /**
   * Helper for adding statistic reporter values
   *
   * @param r_name ReporterName of the data from which the statistics will be computed
   */
  template <typename InType, typename OutType, typename StatType>
  void declareValueHelper(const ReporterName & r_name);
};

template <typename InType, typename OutType, typename StatType>
void
StatisticsReporter::declareValueHelper(const ReporterName & r_name)
{
  const auto & mode = _fe_problem.getReporterData().getReporterMode(r_name);
  const auto & data = getReporterValueByName<InType>(r_name);
  for (const auto & item : _compute_stats)
  {
    const std::string s_name = r_name.getCombinedName() + "_" + item.name();
    if (_ci_method.isValid())
      declareValueByName<StatType, ReporterStatisticsContext<InType, OutType>>(s_name,
                                                                               REPORTER_MODE_ROOT,
                                                                               data,
                                                                               mode,
                                                                               item,
                                                                               _ci_method,
                                                                               _ci_levels,
                                                                               _ci_replicates,
                                                                               _ci_seed);
    else
      declareValueByName<StatType, ReporterStatisticsContext<InType, OutType>>(
          s_name, REPORTER_MODE_ROOT, data, mode, item);
  }
}
