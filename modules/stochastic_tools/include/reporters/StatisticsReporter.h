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
template <typename T>
class ReporterStatisticsContext : public ReporterContext<T>
{
public:
  ReporterStatisticsContext(const libMesh::ParallelObject & other,
                            ReporterState<T> & state,
                            const std::vector<T> & data,
                            const ReporterProducerEnum & mode,
                            const MooseEnumItem & stat,
                            const StochasticTools::BootstrapCalculator * ci_calc);
  virtual void finalize() override;
  virtual void store(nlohmann::json & json) const override;

private:
  /// Data used for the statistic calculation
  const std::vector<T> & _data;

  /// Mode in which the above data was produced
  const ReporterProducerEnum & _data_mode;

  /// Storage for the Calculator object for the desired stat, this is created in constructor
  std::unique_ptr<const StochasticTools::Calculator> _calc_ptr;

  /// Storage for the BootstrapCalculator for the desired confidence interval calculations (optional)
  const StochasticTools::BootstrapCalculator * _ci_calc_ptr;

  /// The results
  std::vector<T> _ci_results;
};

template <typename T>
ReporterStatisticsContext<T>::ReporterStatisticsContext(
    const libMesh::ParallelObject & other,
    ReporterState<T> & state,
    const std::vector<T> & data,
    const ReporterProducerEnum & mode,
    const MooseEnumItem & stat,
    const StochasticTools::BootstrapCalculator * ci_calc)
  : ReporterContext<T>(other, state),
    _data(data),
    _data_mode(mode),
    _calc_ptr(StochasticTools::makeCalculator(stat, other)),
    _ci_calc_ptr(ci_calc)
{
}

template <typename T>
void
ReporterStatisticsContext<T>::finalize()
{
  this->_state.value() = _calc_ptr->compute(_data, _data_mode == REPORTER_MODE_DISTRIBUTED);
  ReporterContext<T>::finalize();

  if (_ci_calc_ptr)
    _ci_results = _ci_calc_ptr->compute(_data, *_calc_ptr, _data_mode == REPORTER_MODE_DISTRIBUTED);
}

template <typename T>
void
ReporterStatisticsContext<T>::store(nlohmann::json & json) const
{
  ReporterContext<T>::store(json);
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

protected:
  /// Confidence level calculator, this is shared by all reporters that are declared.
  std::unique_ptr<const StochasticTools::BootstrapCalculator> _ci_calculator = nullptr;

private:
  /**
   * Helper function for converting confidence levels given in (0, 0.5] into levels in (0, 1).
   * For example, levels_in = {0.05, 0.1, 0.5} converts to {0.05 0.1, 0.5, 0.9, 0.95}.
   *
   * This also performs error checking on the supplied "ci_levels".
   */
  std::vector<Real> computeLevels(const std::vector<Real> & levels_in) const;
};
