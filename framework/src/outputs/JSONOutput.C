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

registerMooseObjectAliased("MooseApp", JSONOutput, "JSON");

InputParameters
JSONOutput::validParams()
{
  InputParameters params = AdvancedOutput::validParams();
  params += AdvancedOutput::enableOutputTypes("system_information reporter");
  params.addClassDescription("Output for Reporter values using JSON format.");
  params.set<ExecFlagEnum>("execute_system_information_on", /*quite_mode=*/true) = EXEC_INITIAL;
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
  const bool has_distributed = _reporter_data.hasReporterWithMode(REPORTER_MODE_DISTRIBUTED);
  if (processor_id() == 0 || has_distributed)
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
    if (n_processors() > 1 && has_distributed)
    {
      _json["part"] = processor_id();
      _json["number_of_parts"] = n_processors();
    }

    // Add Reporter values to the current node
    _reporter_data.store(current_node["reporters"]);
  }
}

void
JSONOutput::output(const ExecFlagType & type)
{
  AdvancedOutput::output(type);

  if (processor_id() == 0 || _reporter_data.hasReporterWithMode(REPORTER_MODE_DISTRIBUTED))
  {
    std::ofstream out(filename().c_str());
    out << std::setw(4) << _json << std::endl;
  }
}
