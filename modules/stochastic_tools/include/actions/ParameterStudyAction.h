//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "hit.h"

// Forward declarations
class ReporterName;

class ParameterStudyAction : public Action
{
public:
  static InputParameters validParams();

  ParameterStudyAction(const InputParameters & params);

  virtual void act() override;

  /// Return an enum of available sampling types for the study
  static MooseEnum samplingTypes();
  /// Return an enum of available distributions for the study
  static MultiMooseEnum distributionTypes();

protected:
  /**
   * The perscribed name of the distribution. Need this to give the right name to the sampler
   *
   * @param count The index of the distribution
   * @return Name of the distribution
   */
  DistributionName distributionName(unsigned int count) const;
  std::vector<DistributionName> distributionNames(unsigned int full_count) const;

  /**
   * The perscribed name of the sampler created in this action.
   *
   * @return SamplerName The name of sampler.
   */
  SamplerName samplerName() const { return "study_sampler"; }

  /**
   * The perscribed name of the multiapp created in this action.
   *
   * @return MultiAppName The name of multiapp.
   */
  MultiAppName multiappName() const { return "study_app"; }

  /**
   * The perscribed name of the control given to the sub-app for parameter transfer.
   *
   * @return std::string The name of the SamplerReceiver control.
   */
  std::string samplerReceiverName() const { return "study_receiver"; }

  /**
   * The perscribed name of the command-line control created in this action.
   *
   * @return std::string The name of the SamplerReceiver control.
   */
  std::string multiappControlName() const { return "study_multiapp_control"; }

  /**
   * The perscribed name of the parameter transfer created in this action.
   *
   * @return std::string The name of the SamplerParameterTransfer transfer.
   */
  std::string parameterTransferName() const { return "study_parameter_transfer"; }

  /**
   * The perscribed name of the reporter transfer created in this action.
   *
   * @return std::string The name of the SamplerReporterTransfer transfer.
   */
  std::string reporterTransferName() const { return "study_qoi_transfer"; }

  /**
   * The perscribed name of the QoI storage object created in this action.
   *
   * @return std::string The name of the StochasticReporter.
   */
  std::string stochasticReporterName() const { return "study_results"; }

  /**
   * The perscribed name of the output objects created in this action.
   *
   * @param type The type of output object created
   * @return std::string The name of the StochasticReporter.
   */
  OutputName outputName(std::string type) const { return type; }

  /**
   * The name of the reporter values in the StochasticReporter representing the QoIs.
   *
   * @param qoi The name of the reporter on the sub-app
   * @return ReporterName The name of the QoI in StochasticReporter.
   */
  ReporterName quantityOfInterestName(const ReporterName & qoi) const;

  /**
   * The perscribed name of the statistics object created in this action.
   *
   * @return std::string The name of the StatisticsReporter.
   */
  std::string statisticsName() const { return "study_statistics"; }

  /**
   * Helper function for getting the param value for the distribution index
   *
   * @tparam T The type of parameter, it will be trying to get std::vector<T>
   * @param param The name of the parameter
   * @param count The index of the specific distribution
   * @return T The inputted value of the parameter at the given index
   */
  template <typename T>
  T getDistributionParam(std::string param, unsigned int count) const;

  /**
   * Helper function to show the object being built. Will display:
   *   - Object base type
   *   - Object type
   *   - Object name
   *   - Parameters set by the action
   *
   * @param type The type of objecte, i.e. "SamplerFullSolveMultiApp"
   * @param name The name of the object
   * @param params The parameters used to create the object
   */
  void showObject(std::string type, std::string name, const InputParameters & params) const;

private:
  /**
   * This is a vector associating the sampling type with a list of associated parameters
   * The list includes the parameter name and whether or not it is required.
   */
  static std::vector<std::map<std::string, bool>> samplerParameters();

  /**
   * This is a vector associating the distribution type and a list of parameters that are needed
   */
  static std::vector<std::vector<std::string>> distributionParameters();

  /**
   * List of parameters that are only associated with computing statistics
   */
  static std::set<std::string> statisticsParameters();

  /**
   * This function will infer the best way to run the multiapps
   *
   * @return unsigned int The multiapp execution mode, see the 'multiapp_mode' input param for
   * details
   */
  unsigned int inferMultiAppMode();

  /// The inputted parameter vector
  const std::vector<std::string> & _parameters;
  /// The sampling type
  const unsigned int _sampling_type;
  /// The distributions
  const MultiMooseEnum _distributions;
  /// The multiapp mode. This is used for determining type of execution for the multiapp and the
  /// way to send the perturbed parameters
  const unsigned int _multiapp_mode;
  /// Whether or not we are computing statistics
  const bool _compute_stats;
  /// Switch to show the objects being built on console
  const bool _show_objects;
};

template <typename T>
T
ParameterStudyAction::getDistributionParam(std::string param, unsigned int count) const
{
  const auto & val = getParam<std::vector<T>>(param);
  return val[count];
}

/**
 * This class is a hit walker used to see if a list of parameters are all controllable
 */
class AreParametersControllableWalker : public hit::Walker
{
public:
  AreParametersControllableWalker(const std::vector<std::string> & parameters, MooseApp & app);

  void walk(const std::string & fullpath, const std::string & nodename, hit::Node * n) override;
  bool areControllable() const;

private:
  MooseApp & _app;
  std::vector<bool> _is_controllable;
  std::vector<std::pair<std::string, std::string>> _pars;
};

/**
 * This class is a hit walker used to see what type of execution the input is doing
 */
class ExecutionTypeWalker : public hit::Walker
{
public:
  ExecutionTypeWalker() : _exec_type(0), _found_exec(false) {}

  void walk(const std::string & fullpath, const std::string & nodename, hit::Node * n) override;
  unsigned int getExecutionType() const { return _exec_type; }

private:
  unsigned int _exec_type;
  bool _found_exec;
};
