//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParameterStudyAction.h"

#include "StochasticToolsAction.h"
#include "FEProblemBase.h"
#include "Control.h"
#include "Calculators.h"

registerMooseAction("StochasticToolsApp", ParameterStudyAction, "meta_action");
registerMooseAction("StochasticToolsApp", ParameterStudyAction, "add_distribution");
registerMooseAction("StochasticToolsApp", ParameterStudyAction, "add_sampler");
registerMooseAction("StochasticToolsApp", ParameterStudyAction, "add_multi_app");
registerMooseAction("StochasticToolsApp", ParameterStudyAction, "add_transfer");
registerMooseAction("StochasticToolsApp", ParameterStudyAction, "add_output");
registerMooseAction("StochasticToolsApp", ParameterStudyAction, "add_reporter");
registerMooseAction("StochasticToolsApp", ParameterStudyAction, "add_control");

InputParameters
ParameterStudyAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Builds objects to set up a basic parameter study.");

  // Parameters to define what we are studying
  params.addRequiredParam<FileName>(
      "input", "The input file containing the physics for the parameter study.");
  params.addRequiredParam<std::vector<std::string>>(
      "parameters", "List of parameters being perturbed for the study.");
  params.addParam<std::vector<ReporterName>>(
      "quantities_of_interest",
      "List of the reporter names (object_name/value_name) "
      "that represent the quantities of interest for the study.");

  // Statistics Parameters
  params.addParam<bool>(
      "compute_statistics",
      true,
      "Whether or not to compute statistics on the 'quantities_of_interest'. "
      "The default is to compute mean and standard deviation with 0.01, 0.05, 0.1, 0.9, "
      "0.95, and 0.99 confidence intervals.");
  MultiMooseEnum stats = StochasticTools::makeCalculatorEnum();
  stats = "mean stddev";
  params.addParam<MultiMooseEnum>(
      "statistics", stats, "The statistic(s) to compute for the study.");
  params.addParam<std::vector<Real>>("ci_levels",
                                     std::vector<Real>({0.01, 0.05, 0.1, 0.9, 0.95, 0.99}),
                                     "A vector of confidence levels to consider for statistics "
                                     "confidence intervals, values must be in (0, 1).");
  params.addParam<unsigned int>(
      "ci_replicates",
      1000,
      "The number of replicates to use when computing confidence level intervals for statistics.");

  // Parameters for the sampling scheme
  params.addRequiredParam<MooseEnum>(
      "sampling_type", samplingTypes(), "The type of sampling to use for the parameter study.");
  params.addParam<unsigned int>("seed", 0, "Random number generator initial seed");

  // Parameters for multi app
  MooseEnum modes(
      "normal=0 batch-reset=1 batch-restore=2 batch-keep-solution=3 batch-no-restore=4");
  params.addParam<MooseEnum>(
      "multiapp_mode",
      modes,
      "The operation mode, 'normal' creates one sub-application for each sample."
      "'batch' creates one sub-app for each processor and re-executes for each local sample. "
      "'reset' re-initializes the sub-app for every sample in the batch. "
      "'restore' does not re-initialize and instead restores to first sample's initialization. "
      "'keep-solution' re-uses the solution obtained from the first sample in the batch. "
      "'no-restore' does not restore the sub-app."
      "The default will be inferred based on the study.");
  params.addParam<unsigned int>(
      "min_procs_per_sample",
      1,
      "Minimum number of processors to give to each sample. Useful for larger, distributed mesh "
      "solves where there are memory constraints.");
  params.addParam<bool>("ignore_solve_not_converge",
                        false,
                        "True to continue main app even if a sub app's solve does not converge.");

  // Samplers ///////////////////////////
  // Parameters for Monte Carlo and LHS
  params.addParam<dof_id_type>(
      "num_samples", "The number of samples to generate for 'monte-carlo' and 'lhs' sampling.");
  params.addParam<MultiMooseEnum>(
      "distributions",
      distributionTypes(),
      "The types of distribution to use for 'monte-carlo' and "
      "'lhs' sampling. The number of entries defines the number of columns in the matrix.");
  // Parameters for cartesian product
  params.addParam<std::vector<Real>>(
      "linear_space_items",
      "Parameter for defining the 'cartesian-prodcut' sampling scheme. A list of triplets, each "
      "item should include the min, step size, and number of steps.");
  // Parameters for CSV sampler
  params.addParam<FileName>(
      "csv_samples_file",
      "Name of the CSV file that contains the sample matrix for 'csv' sampling.");
  params.addParam<std::vector<dof_id_type>>(
      "csv_column_indices",
      "Column indices in the CSV file to be sampled from for 'csv' sampling. Number of indices "
      "here "
      "will be the same as the number of columns per matrix.");
  params.addParam<std::vector<std::string>>(
      "csv_column_names",
      "Column names in the CSV file to be sampled from for 'csv' sampling. Number of columns names "
      "here will be the same as the number of columns per matrix.");
  // Parameters for input matrix
  params.addParam<RealEigenMatrix>("input_matrix", "Sampling matrix for 'input-matrix' sampling.");

  // Distributions ///////////////////////////
  // Parameters for normal distributions
  params.addParam<std::vector<Real>>("normal_mean",
                                     "Means (or expectations) of the 'normal' distributions.");
  params.addParam<std::vector<Real>>("normal_standard_deviation",
                                     "Standard deviations of the 'normal' distributions.");
  // Parameters for uniform distributions
  params.addParam<std::vector<Real>>("uniform_lower_bound",
                                     "Lower bounds for 'uniform' distributions.");
  params.addParam<std::vector<Real>>("uniform_upper_bound",
                                     "Upper bounds 'uniform' distributions.");
  // Parameters for Weibull distributions
  params.addParam<std::vector<Real>>("weibull_location",
                                     "Location parameter (a or low) for 'weibull' distributions.");
  params.addParam<std::vector<Real>>("weibull_scale",
                                     "Scale parameter (b or lambda) for 'weibull' distributions.");
  params.addParam<std::vector<Real>>("weibull_shape",
                                     "Shape parameter (c or k) for 'weibull' distributions.");
  // Parameters for lognormal distributions
  params.addParam<std::vector<Real>>(
      "lognormal_location", "The 'lognormal' distributions' location parameter (m or mu).");
  params.addParam<std::vector<Real>>(
      "lognormal_scale", "The 'lognormal' distributions' scale parameter (s or sigma).");
  // Parameters for truncated normal distributions
  params.addParam<std::vector<Real>>("tnormal_mean",
                                     "Means (or expectations) of the 'tnormal' distributions.");
  params.addParam<std::vector<Real>>("tnormal_standard_deviation",
                                     "Standard deviations of the 'tnormal' distributions.");
  params.addParam<std::vector<Real>>("tnormal_lower_bound", "'tnormal' distributions' lower bound");
  params.addParam<std::vector<Real>>("tnormal_upper_bound", "'tnormal' distributions' upper bound");

  // Outputting parameters
  MultiMooseEnum out_type("none=0 csv=1 json=2", "json");
  params.addParam<MultiMooseEnum>(
      "output_type",
      out_type,
      "Method in which to output sampler matrix and quantities of interest. Warning: "
      "'csv' output will not include vector-type quantities.");
  params.addParam<std::vector<ReporterValueName>>(
      "sampler_column_names",
      "Names of the sampler columns for outputting the sampling matrix. If 'parameters' are not "
      "bracketed, the default is based on these values. Otherwise, the default is based on the "
      "sampler name.");

  // Debug parameters
  params.addParam<bool>("show_study_objects",
                        false,
                        "Set to true to show all the objects being built by this action.");
  return params;
}

ParameterStudyAction::ParameterStudyAction(const InputParameters & parameters)
  : Action(parameters),
    _parameters(getParam<std::vector<std::string>>("parameters")),
    _sampling_type(getParam<MooseEnum>("sampling_type")),
    _distributions(isParamValid("distributions") ? getParam<MultiMooseEnum>("distributions")
                                                 : MultiMooseEnum("")),
    _multiapp_mode(inferMultiAppMode()),
    _compute_stats(isParamValid("quantities_of_interest") && getParam<bool>("compute_statistics")),
    _show_objects(getParam<bool>("show_study_objects"))
{
  // Check sampler parameters
  const auto sampler_params = samplerParameters();
  const auto this_sampler_params = sampler_params[_sampling_type];
  // Check required sampler parameters
  for (const auto & param : this_sampler_params)
    if (param.second && !isParamValid(param.first))
      paramError("sampling_type",
                 "The ",
                 param.first,
                 " parameter is required to build the requested sampling type.");
  // Check unused parameters
  std::string msg = "";
  for (unsigned int i = 0; i < sampler_params.size(); ++i)
    for (const auto & param : sampler_params[i])
      if (this_sampler_params.find(param.first) == this_sampler_params.end() &&
          parameters.isParamSetByUser(param.first))
        msg += (msg.empty() ? "" : ", ") + param.first;
  if (!msg.empty())
    paramError("sampling_type",
               "The following parameters are unused for the selected sampling type: ",
               msg);

  // Check distribution parameters
  const auto distribution_params = distributionParameters();
  std::vector<unsigned int> dist_count(distribution_params.size(), 0);
  for (const auto & dist : _distributions)
    dist_count[(unsigned int)dist]++;
  msg = "";
  for (unsigned int i = 0; i < distribution_params.size(); ++i)
    for (const auto & param : distribution_params[i])
    {
      // Check if parameter was set
      if (dist_count[i] > 0 && !isParamValid(param))
        paramError("distributions",
                   "The ",
                   param,
                   " parameter is required to build the listed distributions.");
      // Check if parameter has correct size
      else if (dist_count[i] > 0 && getParam<std::vector<Real>>(param).size() != dist_count[i])
        paramError("distributions",
                   "The number of entries in ",
                   param,
                   " does not match the number of required entries (",
                   dist_count[i],
                   ") to build the listed distributions.");
      // Check if parameter was set and unused
      else if (dist_count[i] == 0 && parameters.isParamSetByUser(param))
        msg += (msg.empty() ? "" : ", ") + param;
    }
  if (!msg.empty())
    paramError(
        "distributions", "The following parameters are unused for the listed distributions: ", msg);

  // Check statistics parameters
  if (!_compute_stats)
  {
    msg = "";
    for (const auto & param : statisticsParameters())
      if (parameters.isParamSetByUser(param))
        msg += (msg.empty() ? "" : ", ") + param;
    if (!msg.empty())
      paramError("compute_statistics",
                 "The following parameters are unused since statistics are not being computed: ",
                 msg);
  }
}

MooseEnum
ParameterStudyAction::samplingTypes()
{
  return MooseEnum("monte-carlo=0 lhs=1 cartesian-product=2 csv=3 input-matrix=4");
}

MultiMooseEnum
ParameterStudyAction::distributionTypes()
{
  return MultiMooseEnum("normal=0 uniform=1 weibull=2 lognormal=3 tnormal=4");
}

void
ParameterStudyAction::act()
{
  if (_current_task == "meta_action")
  {
    const auto stm_actions = _awh.getActions<StochasticToolsAction>();
    if (stm_actions.empty())
    {
      auto params = _action_factory.getValidParams("StochasticToolsAction");
      params.set<bool>("_built_by_moose") = true;
      params.set<std::string>("registered_identifier") = "(AutoBuilt)";

      std::shared_ptr<Action> action = _action_factory.create(
          "StochasticToolsAction", _name + "_stochastic_tools_action", params);
      _awh.addActionBlock(action);

      if (_show_objects)
        showObject("StochasticToolsAction", _name + "_stochastic_tools_action", params);
    }
  }
  else if (_current_task == "add_distribution")
  {
    // This map is used to keep track of how many of a certain
    // distribution is being created.
    std::unordered_map<std::string, unsigned int> dist_count;
    for (const auto & dt : distributionTypes().getNames())
      dist_count[dt] = 0;

    // We will have a single call to addDistribution for each entry
    // So declare these quantities and set in the if statements
    std::string distribution_type;
    InputParameters params = emptyInputParameters();

    // Loop through the inputted distributions
    unsigned int full_count = 0;
    for (const auto & dist : _distributions)
    {
      // Convenient reference to the current count
      unsigned int & count = dist_count[dist.name()];

      // Set the distribution type and parameters
      if (dist == "normal")
      {
        distribution_type = "Normal";
        params = _factory.getValidParams(distribution_type);
        params.set<Real>("mean") = getDistributionParam<Real>("normal_mean", count);
        params.set<Real>("standard_deviation") =
            getDistributionParam<Real>("normal_standard_deviation", count);
      }
      else if (dist == "uniform")
      {
        distribution_type = "Uniform";
        params = _factory.getValidParams(distribution_type);
        params.set<Real>("lower_bound") = getDistributionParam<Real>("uniform_lower_bound", count);
        params.set<Real>("upper_bound") = getDistributionParam<Real>("uniform_upper_bound", count);
      }
      else if (dist == "weibull")
      {
        distribution_type = "Weibull";
        params = _factory.getValidParams(distribution_type);
        params.set<Real>("location") = getDistributionParam<Real>("weibull_location", count);
        params.set<Real>("scale") = getDistributionParam<Real>("weibull_scale", count);
        params.set<Real>("shape") = getDistributionParam<Real>("weibull_shape", count);
      }
      else if (dist == "lognormal")
      {
        distribution_type = "Lognormal";
        params = _factory.getValidParams(distribution_type);
        params.set<Real>("location") = getDistributionParam<Real>("lognormal_location", count);
        params.set<Real>("scale") = getDistributionParam<Real>("lognormal_scale", count);
      }
      else if (dist == "tnormal")
      {
        distribution_type = "TruncatedNormal";
        params = _factory.getValidParams(distribution_type);
        params.set<Real>("mean") = getDistributionParam<Real>("tnormal_mean", count);
        params.set<Real>("standard_deviation") =
            getDistributionParam<Real>("tnormal_standard_deviation", count);
        params.set<Real>("lower_bound") = getDistributionParam<Real>("tnormal_lower_bound", count);
        params.set<Real>("upper_bound") = getDistributionParam<Real>("tnormal_upper_bound", count);
      }
      else
        paramError("distributions", "Unknown distribution type.");

      // Add the distribution
      _problem->addDistribution(distribution_type, distributionName(full_count), params);
      if (_show_objects)
        showObject(distribution_type, distributionName(full_count), params);

      // Increment the counts
      count++;
      full_count++;
    }
  }
  else if (_current_task == "add_sampler")
  {
    // We will have a single call to addSampler
    // So declare these quantities and set in the if statements
    std::string sampler_type;
    InputParameters params = emptyInputParameters();

    // Set the distribution type and parameters
    // monte-carlo or lhs
    if (_sampling_type == 0 || _sampling_type == 1)
    {
      sampler_type = _sampling_type == 0 ? "MonteCarlo" : "LatinHypercube";
      params = _factory.getValidParams(sampler_type);
      params.set<dof_id_type>("num_rows") = getParam<dof_id_type>("num_samples");
      params.set<std::vector<DistributionName>>("distributions") =
          distributionNames(_distributions.size());
    }
    // cartesian-product
    else if (_sampling_type == 2)
    {
      sampler_type = "CartesianProduct";
      params = _factory.getValidParams(sampler_type);
      params.set<std::vector<Real>>("linear_space_items") =
          getParam<std::vector<Real>>("linear_space_items");
    }
    // csv
    else if (_sampling_type == 3)
    {
      sampler_type = "CSVSampler";
      params = _factory.getValidParams(sampler_type);
      params.set<FileName>("samples_file") = getParam<FileName>("csv_samples_file");
      if (isParamValid("csv_column_indices") && isParamValid("csv_column_names"))
        paramError("csv_column_indices",
                   "'csv_column_indices' and 'csv_column_names' cannot both be set.");
      else if (isParamValid("csv_column_indices"))
        params.set<std::vector<dof_id_type>>("column_indices") =
            getParam<std::vector<dof_id_type>>("csv_column_indices");
      else if (isParamValid("csv_column_names"))
        params.set<std::vector<std::string>>("column_names") =
            getParam<std::vector<std::string>>("csv_column_names");
    }
    // input-matrix
    else if (_sampling_type == 4)
    {
      sampler_type = "InputMatrix";
      params = _factory.getValidParams(sampler_type);
      params.set<RealEigenMatrix>("matrix") = getParam<RealEigenMatrix>("input_matrix");
    }
    else
      paramError("sampling_type", "Unknown sampling type.");

    // Need to set the right execute_on for command-line control
    if (_multiapp_mode <= 1)
      params.set<ExecFlagEnum>("execute_on") = {EXEC_PRE_MULTIAPP_SETUP};
    else
      params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL};

    // Set the minimum number of procs
    params.set<unsigned int>("min_procs_per_row") = getParam<unsigned int>("min_procs_per_sample");

    // Add the sampler
    _problem->addSampler(sampler_type, samplerName(), params);
    if (_show_objects)
      showObject(sampler_type, samplerName(), params);
  }
  else if (_current_task == "add_multi_app")
  {
    auto params = _factory.getValidParams("SamplerFullSolveMultiApp");

    // Dealing with failed solves
    params.set<bool>("ignore_solve_not_converge") = getParam<bool>("ignore_solve_not_converge");

    // Set input file
    params.set<std::vector<FileName>>("input_files") = {getParam<FileName>("input")};

    // Set the Sampler
    params.set<SamplerName>("sampler") = samplerName();

    // Set parameters based on the sampling mode
    // normal
    if (_multiapp_mode == 0)
      params.set<MooseEnum>("mode") = "normal";
    // batch-reset
    else if (_multiapp_mode == 1)
      params.set<MooseEnum>("mode") = "batch-reset";
    // batch-restore variants
    else
    {
      // Set the mode to 'batch-restore'
      params.set<MooseEnum>("mode") = "batch-restore";

      // If we are doing batch-restore, the parameters must be controllable.
      // So we will add the necessary control to the sub-app using command-line
      std::string clia = "Controls/" + samplerReceiverName() + "/type=SamplerReceiver";
      params.set<std::vector<CLIArgString>>("cli_args").push_back(clia);

      // batch-keep-solution
      if (_multiapp_mode == 3)
        params.set<bool>("keep_solution_during_restore") = true;
      // batch-no-restore
      else if (_multiapp_mode == 4)
        params.set<bool>("no_restore") = true;
    }

    // Set the minimum number of procs
    params.set<unsigned int>("min_procs_per_app") = getParam<unsigned int>("min_procs_per_sample");

    // Setting execute_on to make sure things happen in the correct order
    params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_BEGIN};

    // Add the multiapp
    _problem->addMultiApp("SamplerFullSolveMultiApp", multiappName(), params);
    if (_show_objects)
      showObject("SamplerFullSolveMultiApp", multiappName(), params);
  }
  else if (_current_task == "add_transfer")
  {
    // Add the parameter transfer if we are doing 'batch-restore'
    if (_multiapp_mode >= 2)
    {
      auto params = _factory.getValidParams("SamplerParameterTransfer");
      params.set<MultiAppName>("to_multi_app") = multiappName();
      params.set<SamplerName>("sampler") = samplerName();
      params.set<std::vector<std::string>>("parameters") = _parameters;
      _problem->addTransfer("SamplerParameterTransfer", parameterTransferName(), params);
      if (_show_objects)
        showObject("SamplerParameterTransfer", parameterTransferName(), params);
    }

    // Add reporter transfer if QoIs have been specified
    if (isParamValid("quantities_of_interest"))
    {
      auto params = _factory.getValidParams("SamplerReporterTransfer");
      params.set<MultiAppName>("from_multi_app") = multiappName();
      params.set<SamplerName>("sampler") = samplerName();
      params.set<std::string>("stochastic_reporter") = stochasticReporterName();
      params.set<std::vector<ReporterName>>("from_reporter") =
          getParam<std::vector<ReporterName>>("quantities_of_interest");
      params.set<std::string>("prefix") = "";
      _problem->addTransfer("SamplerReporterTransfer", reporterTransferName(), params);
      if (_show_objects)
        showObject("SamplerReporterTransfer", reporterTransferName(), params);
    }
  }
  else if (_current_task == "add_output")
  {
    const auto & output = getParam<MultiMooseEnum>("output_type");

    // Add csv output
    if (output.isValueSet("csv"))
    {
      auto params = _factory.getValidParams("CSV");
      params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END};
      _problem->addOutput("CSV", outputName("csv"), params);
      if (_show_objects)
        showObject("CSV", outputName("csv"), params);
    }

    // Add json output
    if (output.isValueSet("json") || _compute_stats)
    {
      auto params = _factory.getValidParams("JSON");
      params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END};
      _problem->addOutput("JSON", outputName("json"), params);
      if (_show_objects)
        showObject("JSON", outputName("json"), params);
    }
  }
  else if (_current_task == "add_reporter")
  {
    // Add stochastic reporter object
    auto params = _factory.getValidParams("StochasticMatrix");

    // Ideally this would be based on the number of samples since gathering
    // data onto a single processor can be memory and run-time expensive,
    // but most people want everything in one output file
    params.set<MooseEnum>("parallel_type") = "ROOT";

    // Supply the sampler for output
    params.set<SamplerName>("sampler") = samplerName();

    // Set the column names if supplied or identifiable with "parameters"
    auto & names = params.set<std::vector<ReporterValueName>>("sampler_column_names");
    if (isParamValid("sampler_column_names"))
      names = getParam<std::vector<ReporterValueName>>("sampler_column_names");
    else
    {
      // There isn't a guaranteed mapping if using brackets
      bool has_bracket = false;
      for (const auto & param : _parameters)
        if (param.find("[") != std::string::npos)
          has_bracket = true;

      // If no brackets, then there is mapping, so use parameter names
      if (!has_bracket)
        for (auto param : _parameters)
        {
          // Reporters don't like '/' in the name, so replace those with '_'
          std::replace(param.begin(), param.end(), '/', '_');
          names.push_back(param);
        }
    }

    // Specify output objects
    const auto & output_type = getParam<MultiMooseEnum>("output_type");
    auto & outputs = params.set<std::vector<OutputName>>("outputs");
    if (output_type.isValueSet("csv"))
      outputs.push_back(outputName("csv"));
    if (output_type.isValueSet("json"))
      outputs.push_back(outputName("json"));
    if (output_type.isValueSet("none"))
      outputs = {"none"};

    params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END};
    _problem->addReporter("StochasticMatrix", stochasticReporterName(), params);
    if (_show_objects)
      showObject("StochasticReporter", stochasticReporterName(), params);

    // Add statistics object
    if (_compute_stats)
    {
      auto params = _factory.getValidParams("StatisticsReporter");
      auto & reps = params.set<std::vector<ReporterName>>("reporters");
      for (const auto & qoi : getParam<std::vector<ReporterName>>("quantities_of_interest"))
        reps.push_back(quantityOfInterestName(qoi));
      params.set<MultiMooseEnum>("compute") = getParam<MultiMooseEnum>("statistics");
      params.set<MooseEnum>("ci_method") = "percentile";
      params.set<std::vector<Real>>("ci_levels") = getParam<std::vector<Real>>("ci_levels");
      params.set<unsigned int>("ci_replicates") = getParam<unsigned int>("ci_replicates");
      params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_END};
      params.set<std::vector<OutputName>>("outputs") = {outputName("json")};
      _problem->addReporter("StatisticsReporter", statisticsName(), params);
      if (_show_objects)
        showObject("StatisticsReporter", statisticsName(), params);
    }
  }
  else if (_current_task == "add_control")
  {
    // Add command-line control if the multiapp mode warrants it
    if (_multiapp_mode <= 1)
    {
      auto params = _factory.getValidParams("MultiAppSamplerControl");
      params.set<MultiAppName>("multi_app") = multiappName();
      params.set<SamplerName>("sampler") = samplerName();
      params.set<std::vector<std::string>>("param_names") = _parameters;
      auto control =
          _factory.create<Control>("MultiAppSamplerControl", multiappControlName(), params);
      _problem->getControlWarehouse().addObject(control);
      if (_show_objects)
        showObject("MultiAppSamplerControl", multiappControlName(), params);
    }
  }
}

DistributionName
ParameterStudyAction::distributionName(unsigned int count) const
{
  return "study_distribution_" + std::to_string(count);
}

std::vector<DistributionName>
ParameterStudyAction::distributionNames(unsigned int full_count) const
{
  std::vector<DistributionName> dist_names;
  for (const auto & i : make_range(full_count))
    dist_names.push_back(distributionName(i));
  return dist_names;
}

ReporterName
ParameterStudyAction::quantityOfInterestName(const ReporterName & qoi) const
{
  return ReporterName(stochasticReporterName(), qoi.getObjectName() + ":" + qoi.getValueName());
}

void
ParameterStudyAction::showObject(std::string type,
                                 std::string name,
                                 const InputParameters & params) const
{
  // Output basic information
  std::string base_type = params.have_parameter<std::string>("_moose_base")
                              ? params.get<std::string>("_moose_base")
                              : "Unknown";
  _console << "[ParameterStudy] "
           << "Base Type:  " << COLOR_YELLOW << base_type << COLOR_DEFAULT << "\n"
           << "                 Type:       " << COLOR_YELLOW << type << COLOR_DEFAULT << "\n"
           << "                 Name:       " << COLOR_YELLOW << name << COLOR_DEFAULT;

  // Gather parameters and their values if:
  // - It is not a private parameter
  // - Doesn't start with '_' (which usually indicates private)
  // - Parameter is set by this action
  // - Is not the "type" parameter
  std::map<std::string, std::string> param_map;
  for (const auto & it : params)
    if (!params.isPrivate(it.first) && it.first[0] != '_' && params.isParamSetByUser(it.first) &&
        it.first != "type")
    {
      std::stringstream ss;
      it.second->print(ss);
      param_map[it.first] = ss.str();
    }

  // Print the gathered parameters
  if (!param_map.empty())
    _console << "\n                 Parameters: ";
  bool first = true;
  for (const auto & it : param_map)
  {
    if (!first)
      _console << "\n" << std::string(29, ' ');
    _console << COLOR_YELLOW << std::setw(24) << it.first << COLOR_DEFAULT << " : " << COLOR_MAGENTA
             << it.second << COLOR_DEFAULT;
    first = false;
  }

  _console << std::endl;
}

std::vector<std::map<std::string, bool>>
ParameterStudyAction::samplerParameters()
{
  // monte-carlo, lhs, cartesian-product, csv, input-matrix
  return {{{"num_samples", true}, {"distributions", true}},
          {{"num_samples", true}, {"distributions", true}},
          {{"linear_space_items", true}},
          {{"csv_samples_file", true}, {"csv_column_indices", false}, {"csv_column_names", false}},
          {{"input_matrix", true}}};
}

std::vector<std::vector<std::string>>
ParameterStudyAction::distributionParameters()
{
  // normal, uniform, weibull, lognormal, tnormal
  return {
      {"normal_mean", "normal_standard_deviation"},
      {"uniform_lower_bound", "uniform_upper_bound"},
      {"weibull_location", "weibull_scale", "weibull_shape"},
      {"lognormal_location", "lognormal_scale"},
      {"tnormal_mean", "tnormal_standard_deviation", "tnormal_lower_bound", "tnormal_upper_bound"}};
}

std::set<std::string>
ParameterStudyAction::statisticsParameters()
{
  return {"statistics", "ci_levels", "ci_replicates"};
}

unsigned int
ParameterStudyAction::inferMultiAppMode()
{
  if (isParamValid("multiapp_mode"))
    return getParam<MooseEnum>("multiapp_mode");

  const unsigned int default_mode = 1;

  // First obvious thing is if it is a parsed parameter, indicated by the lack of '/'
  for (const auto & param : _parameters)
    if (param.find("/") == std::string::npos)
      return default_mode;
  // Next we'll see if there is a GlobalParam
  for (const auto & param : _parameters)
    if (param.find("GlobalParams") != std::string::npos)
      return default_mode;

  // Now for the difficult check
  // Parse input file and create root hit node
  const auto input_filename = MooseUtils::realpath(getParam<FileName>("input"));
  std::ifstream f(input_filename);
  std::string input((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  std::unique_ptr<hit::Node> root(hit::parse(input_filename, input));
  hit::explode(root.get());

  // Walk through the input and see if every param is controllable
  AreParametersControllableWalker control_walker(_parameters, _app);
  root->walk(&control_walker, hit::NodeType::Section);
  if (!control_walker.areControllable())
    return default_mode;

  // Walk through the input and determine how the problem is being executed
  ExecutionTypeWalker exec_walker;
  root->walk(&exec_walker, hit::NodeType::Section);
  // If it is steady-state, then we don't need to restore
  if (exec_walker.getExecutionType() == 1)
    return 4;
  // If it is pseudo-transeint, then we can keep the solution
  else if (exec_walker.getExecutionType() == 2)
    return 3;
  // If it's transient or unknown, don't keep the solution and restore
  else
    return 2;
}

AreParametersControllableWalker::AreParametersControllableWalker(
    const std::vector<std::string> & parameters, MooseApp & app)
  : _app(app), _is_controllable(parameters.size(), false)
{
  // Seperate the object from the parameter into a list of pairs
  for (const auto & param : parameters)
  {
    const auto pos = param.rfind("/");
    _pars.emplace_back(param.substr(0, pos), param.substr(pos + 1));
  }
}

void
AreParametersControllableWalker::walk(const std::string & fullpath,
                                      const std::string & /*nodename*/,
                                      hit::Node * n)
{
  for (const auto & i : index_range(_pars))
  {
    const std::string obj = _pars[i].first;
    const std::string par = _pars[i].second;
    if (obj == fullpath)
    {
      const auto typeit = n->find("type");
      if (typeit && typeit != n && typeit->type() == hit::NodeType::Field)
      {
        const std::string obj_type = n->param<std::string>("type");
        const auto params = _app.getFactory().getValidParams(obj_type);
        _is_controllable[i] = params.isControllable(par);
      }
    }
  }
}

bool
AreParametersControllableWalker::areControllable() const
{
  for (const auto & ic : _is_controllable)
    if (!ic)
      return false;
  return true;
}

void
ExecutionTypeWalker::walk(const std::string & fullpath,
                          const std::string & /*nodename*/,
                          hit::Node * n)
{
  if (fullpath == "Executioner")
  {
    // This should not be hit since there shouldn't be two Executioner blocks
    // But if it does happen, then go back to not knowing
    if (_found_exec)
      _exec_type = 0;
    else
    {
      // Get the type of executioner
      std::string executioner_type = "Unknown";
      const auto typeit = n->find("type");
      if (typeit && typeit != n && typeit->type() == hit::NodeType::Field)
        executioner_type = n->param<std::string>("type");

      // If it's Steady or Eigenvalue, then it's a steady-state problem
      if (executioner_type == "Steady" || executioner_type == "Eigenvalue")
        _exec_type = 1;
      // If it's Transient
      else if (executioner_type == "Transient")
      {
        // Now we'll see if it's a pseudo transient
        const auto it = n->find("steady_state_detection");
        if (it && it != n && it->type() == hit::NodeType::Field &&
            n->param<bool>("steady_state_detection"))
          _exec_type = 2;
        else
          _exec_type = 3;
      }
    }

    _found_exec = true;
  }
}
