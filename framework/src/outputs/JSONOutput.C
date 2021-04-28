//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  return params;
}

JSONOutput::JSONOutput(const InputParameters & parameters)
  : AdvancedOutput(parameters), _reporter_data(_problem_ptr->getReporterData())
{
}

std::string
JSONOutput::filename()
{
  if (processor_id() > 0)
  {
    std::ostringstream file_name;
    int digits = MooseUtils::numDigits(n_processors());
    file_name << _file_base << ".json"
              << "." << std::setw(digits) << std::setfill('0') << processor_id();
    return file_name.str();
  }
  return _file_base + ".json";
}

void
JSONOutput::outputSystemInformation()
{
  storeHelper(_json, _app);
}

void
JSONOutput::outputReporters()
{
  // Set of ReporterNames for output
  std::set<ReporterName> r_names;
  for (const std::string & c_name : getReporterOutput())
    r_names.emplace(c_name);

  // Is there ANY distributed data
  _has_distributed = std::any_of(r_names.begin(), r_names.end(), [this](const ReporterName & n) {
    return _reporter_data.hasReporterWithMode(n.getObjectName(), REPORTER_MODE_DISTRIBUTED);
  });
  if (processor_id() == 0 || _has_distributed)
  {
    // Create the current output node
    auto & current_node = _json["time_steps"].emplace_back();

    // Add time/iteration information
    current_node["time"] = _problem_ptr->time();
    current_node["time_step"] = _problem_ptr->timeStep();
    if (_execute_enum.contains(EXEC_LINEAR) && !_on_nonlinear_residual)
      current_node["linear_iteration"] = _linear_iter;
    if (_execute_enum.contains(EXEC_NONLINEAR))
      current_node["nonlinear_iteration"] = _nonlinear_iter;

    // Inject processor info
    if (n_processors() > 1 && _has_distributed)
    {
      _json["part"] = processor_id();
      _json["number_of_parts"] = n_processors();
    }

    // Add Reporter values to the current node
    auto & r_node = current_node["reporters"]; // non-accidental insert
    for (const auto & r_name : r_names)
    {
      // Create/get object node
      auto obj_node_pair = r_node.emplace(r_name.getObjectName(), nlohmann::json());
      auto & obj_node = *(obj_node_pair.first);

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
        mooseAssert(objs.size() <= 1,
                    "Multiple Reporter objects with the same name located, how did you do that?");

        // It is possible to have a Reporter value without a reporter objects (i.e., VPPs, PPs),
        // which is why objs can be empty.
        if (!objs.empty())
          objs.front()->store(obj_node["info"]);
      }

      // Insert reporter value
      auto & node = obj_node["values"].emplace_back();
      _reporter_data.getReporterContextBase(r_name).store(node);
    }
  }
}

void
JSONOutput::output(const ExecFlagType & type)
{
  _has_distributed = false;
  AdvancedOutput::output(type);
  if (processor_id() == 0 || _has_distributed)
  {
    std::ofstream out(filename().c_str());
    out << std::setw(4) << _json << std::endl;
  }
}
