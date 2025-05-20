//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "JSONOutput.h"
#include "FEProblem.h"
#include "MooseApp.h"
#include "JsonIO.h"
#include "Reporter.h"
#include "TheWarehouse.h"
#include "CommonOutputAction.h"

registerMooseObjectAliased("MooseApp", JSONOutput, "JSON");

InputParameters
JSONOutput::validParams()
{
  InputParameters params = AdvancedOutput::validParams();
  params += AdvancedOutput::enableOutputTypes("system_information reporter");
  params.addClassDescription("Output for Reporter values using JSON format.");
  params.set<ExecFlagEnum>("execute_system_information_on", /*quite_mode=*/true) = EXEC_INITIAL;
  params.addParam<bool>(
      "use_legacy_reporter_output", false, "Use reporter output that does not group by object.");
  params.addParam<bool>("one_file_per_timestep",
                        false,
                        "Create a unique output file for each time step of the simulation.");
  params.addParam<std::vector<ReporterName>>(
      "reporters", "Specific reporter values to output; if not set, all will be output");
  params.addParam<bool>(
      "distributed", true, "Whether or not to output distributed data (data not on rank 0)");
  return params;
}

JSONOutput::JSONOutput(const InputParameters & parameters)
  : AdvancedOutput(parameters),
    _reporter_data(_problem_ptr->getReporterData()),
    _one_file_per_timestep(getParam<bool>("one_file_per_timestep")),
    _reporters(getOptionalParam<std::vector<ReporterName>>("reporters")),
    _json(declareRestartableData<nlohmann::json>("json_out_str"))
{
}

void
JSONOutput::initialSetup()
{
  if (_reporters && _reporters->size())
    for (const auto & name : *_reporters)
      if (!_problem_ptr->getReporterData().hasReporterValue(name))
        paramError("reporters", "Reporter value '", name, "' was not found");
}

std::string
JSONOutput::filename()
{
  std::ostringstream file_name;
  file_name << _file_base;

  if (_one_file_per_timestep)
    file_name << '_' << std::setw(_padding) << std::setprecision(0) << std::setfill('0')
              << std::right << timeStep();

  if (processor_id() > 0)
  {
    int digits = MooseUtils::numDigits(n_processors());
    file_name << ".json"
              << "." << std::setw(digits) << std::setfill('0') << processor_id();
  }
  else
    file_name << ".json";

  return file_name.str();
}

void
JSONOutput::outputSystemInformation()
{
  nlohmann::to_json(_json, _app);
}

void
JSONOutput::timestepSetup()
{
  AdvancedOutput::timestepSetup();
  if (_one_file_per_timestep)
    _json.clear();
}

void
JSONOutput::outputReporters()
{
  // Get the reporter values that we should skip. The common output action can
  // add JSON output objects for specific reporters, of which we don't want to
  // include within the standard json output
  std::set<ReporterName> skip_names;
  const auto common_actions = _app.actionWarehouse().getActions<CommonOutputAction>();
  if (common_actions.size())
  {
    mooseAssert(common_actions.size() == 1, "Should not be more than one");
    const auto & action = *common_actions[0];
    const auto & common_names = action.getCommonReporterNames();
    skip_names.insert(common_names.begin(), common_names.end());
  }

  // Set of ReporterNames for output
  std::set<ReporterName> r_names;
  for (const std::string & c_name : getReporterOutput())
  {
    const ReporterName r_name(c_name);

    // If specific values are requested, skip anything that isn't that value
    if (_reporters)
    {
      if (std::find(_reporters->begin(), _reporters->end(), r_name) == _reporters->end())
        continue;
    }
    // If all values are requested, skip those that should be skipped
    else if (skip_names.count(r_name))
      continue;

    r_names.emplace(r_name);
  }

  // Is there ANY distributed data
  _has_distributed = getParam<bool>("distributed") &&
                     std::any_of(r_names.begin(),
                                 r_names.end(),
                                 [this](const ReporterName & n) {
                                   return _reporter_data.hasReporterWithMode(
                                       n.getObjectName(), REPORTER_MODE_DISTRIBUTED);
                                 });
  if (processor_id() == 0 || _has_distributed)
  {
    // Create the current output node
    auto & current_node = _json["time_steps"].emplace_back();

    // Add time/iteration information
    current_node["time"] = _problem_ptr->time();
    current_node["time_step"] = _problem_ptr->timeStep();
    if (_execute_enum.isValueSet(EXEC_LINEAR) && !_on_nonlinear_residual)
      current_node["linear_iteration"] = _linear_iter;
    if (_execute_enum.isValueSet(EXEC_NONLINEAR))
      current_node["nonlinear_iteration"] = _nonlinear_iter;

    // Inject processor info
    if (n_processors() > 1 && _has_distributed)
    {
      _json["part"] = processor_id();
      _json["number_of_parts"] = n_processors();
    }

    // Add Reporter values to the current node
    auto & r_node = _json["reporters"]; // non-accidental insert
    for (const auto & r_name : r_names)
    {
      // If this value is produced in root mode and we're not on root, don't report this value
      const auto & context = _reporter_data.getReporterContextBase(r_name);
      if (context.getProducerModeEnum() == REPORTER_MODE_ROOT && processor_id() != 0)
        continue;

      // Create/get object node
      auto obj_node_pair = r_node.emplace(r_name.getObjectName(), nlohmann::json());
      auto & obj_node = *(obj_node_pair.first);

      // Whether or not we should store this Reporter's value or have it be null
      bool should_store = true;

      // If the object node was created insert the class level information
      if (obj_node_pair.second)
      {
        // Query the TheWarehouse for all Reporter objects with the given name. The attributes and
        // QueryID are used to allow the TheWarehouse::queryInto method be called with the
        // "show_all" option set to true. This returns all objects, regardless of "enabled" state,
        // which is what is needed to ensure that output always happens, even if an object is
        // disabled.
        std::vector<Reporter *> objs;
        auto attr = _problem_ptr->theWarehouse()
                        .query()
                        .condition<AttribInterfaces>(Interfaces::Reporter)
                        .condition<AttribName>(r_name.getObjectName())
                        .attributes();
        auto qid = _problem_ptr->theWarehouse().queryID(attr);
        _problem_ptr->theWarehouse().queryInto(qid, objs, true);

        // There can now be multiple reporter objects with the same name, but
        // there will only be one reporter that stores all the data.
        if (!objs.empty())
        {
          auto & reporter = *objs.front();

          // It is possible to have a Reporter value without a reporter objects (i.e., VPPs, PPs),
          // which is why objs can be empty.
          reporter.store(obj_node);

          // GeneralReporters have the option to only store JSON data when the execute flag
          // matches an execute flag that is within the GeneralReporter; this captures that
          should_store = reporter.shouldStore();
        }
      }

      // Create/get value node
      auto value_node_pair = obj_node["values"].emplace(r_name.getValueName(), nlohmann::json());
      // If the value node was created insert the value information
      if (value_node_pair.second)
        // Store value meta data
        context.storeInfo(*value_node_pair.first);

      // Insert reporter value
      auto & node = current_node[r_name.getObjectName()][r_name.getValueName()];
      if (should_store)
        context.store(node);
      else
        node = "null";
    }
  }
}

void
JSONOutput::output()
{
  _has_distributed = false;
  AdvancedOutput::output();
  if (processor_id() == 0 || _has_distributed)
  {
    std::ofstream out(filename().c_str());
    out << std::setw(4) << _json << std::endl;
  }
}

template <>
void
dataStore(std::ostream & stream, nlohmann::json & json, void * /*context*/)
{
  stream << json;
}

template <>
void
dataLoad(std::istream & stream, nlohmann::json & json, void * /*context*/)
{
  stream >> json;
}
