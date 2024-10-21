//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "CSV.h"
#include "FEProblem.h"
#include "MooseApp.h"

registerMooseObject("MooseApp", CSV);

InputParameters
CSV::validParams()
{
  // Get the parameters from the parent object
  InputParameters params = TableOutput::validParams();
  params.addClassDescription("Output for postprocessors, vector postprocessors, and scalar "
                             "variables using comma seperated values (CSV).");
  params.addParam<bool>("sort_columns", false, "Toggle the sorting of columns alphabetically.");

  // Options for aligning csv output with whitespace padding
  params.addParam<bool>(
      "align",
      false,
      "Align the outputted csv data by padding the numbers with trailing whitespace");
  params.addParam<std::string>("delimiter", ",", "Assign the delimiter (default is ','");
  params.addParam<unsigned int>("precision", 14, "Set the output precision");
  params.addParam<bool>("create_final_symlink",
                        false,
                        "Enable/disable the creation of a _FINAL symlink for vector postprocessor "
                        "data with 'execute_on' includes 'FINAL'.");
  params.addParam<bool>(
      "create_latest_symlink",
      false,
      "Enable/disable the creation of a _LATEST symlink for vector postprocessor data.");

  params.addParamNamesToGroup("sort_columns align delimiter precision", "Table formatting");
  params.addParamNamesToGroup("create_latest_symlink create_final_symlink", "Symbolic links");
  // Suppress unused parameters
  params.suppressParameter<unsigned int>("padding");

  // Done
  return params;
}

CSV::CSV(const InputParameters & parameters)
  : TableOutput(parameters),
    _align(getParam<bool>("align")),
    _precision(getParam<unsigned int>("precision")),
    _delimiter(getParam<std::string>("delimiter")),
    _write_all_table(false),
    _write_vector_table(false),
    _sort_columns(getParam<bool>("sort_columns")),
    _recovering(_app.isRecovering()),
    _create_final_symlink(getParam<bool>("create_final_symlink")),
    _create_latest_symlink(getParam<bool>("create_latest_symlink"))
{
}

void
CSV::initialSetup()
{
  // Call the base class method
  TableOutput::initialSetup();

  // Set the delimiter
  _all_data_table.setDelimiter(_delimiter);

  // Set the precision
  _all_data_table.setPrecision(_precision);

  if (_recovering)
    _all_data_table.append(true);

  // Clear any existing symbolic links to LATEST and/or FINAL
  if (processor_id() == 0)
  {
    const std::set<std::string> & out = getVectorPostprocessorOutput();
    for (const auto & vpp_name : out)
    {
      std::string short_name = MooseUtils::shortName(vpp_name);
      std::string out_latest = _file_base + "_" + short_name + "_LATEST.csv";
      std::string out_final = _file_base + "_" + short_name + "_FINAL.csv";
      MooseUtils::clearSymlink(out_latest);
      MooseUtils::clearSymlink(out_final);
    }
  }

  // See https://github.com/idaholab/moose/issues/25211.
  mooseAssert(advancedExecuteOn().contains("postprocessors"),
              "Missing expected postprocessors key");
  mooseAssert(advancedExecuteOn().contains("scalars"), "Missing expected scalars key");
  mooseAssert(advancedExecuteOn().contains("reporters"), "Missing expected reporters key");
  const auto pp_execute_on = advancedExecuteOn().find("postprocessors")->second;
  const auto scalar_execute_on = advancedExecuteOn().find("scalars")->second;
  const auto reporter_execute_on = advancedExecuteOn().find("reporters")->second;
  const auto n_pps = getPostprocessorOutput().size();
  const auto n_scalars = getScalarOutput().size();
  const auto n_reporters = getReporterOutput().size();
  const bool pp_active = n_pps > 0 && !pp_execute_on.isValueSet(EXEC_NONE);
  const bool scalar_active = n_scalars > 0 && !scalar_execute_on.isValueSet(EXEC_NONE);
  const bool reporter_active = n_reporters > 0 && !reporter_execute_on.isValueSet(EXEC_NONE);
  if ((pp_execute_on != scalar_execute_on && pp_active && scalar_active) ||
      (pp_execute_on != reporter_execute_on && pp_active && reporter_active))
    mooseError("The parameters 'execute_postprocessors_on', 'execute_scalars_on', and "
               "'execute_reporters_on' must be the same for CSV output.");
}

std::string
CSV::filename()
{
  return _file_base + ".csv";
}

void
CSV::outputScalarVariables()
{
  TableOutput::outputScalarVariables();
  _write_all_table = true;
}

void
CSV::outputPostprocessors()
{
  TableOutput::outputPostprocessors();
  _write_all_table = true;
}

void
CSV::outputVectorPostprocessors()
{
  TableOutput::outputVectorPostprocessors();
  _write_vector_table = true;
}

void
CSV::outputReporters()
{
  TableOutput::outputReporters();
  _write_all_table = true;
  _write_vector_table = true;
}

std::string
CSV::getVectorPostprocessorFileName(const std::string & vpp_name,
                                    bool include_time_step,
                                    bool is_distributed)
{
  std::ostringstream file_name;
  file_name << _file_base;

  auto short_name = MooseUtils::shortName(vpp_name);
  if (short_name.size())
    file_name << '_' << short_name;

  if (include_time_step)
  {
    file_name << '_' << std::setw(_padding) << std::setprecision(0) << std::setfill('0')
              << std::right << timeStep();

    if (_current_execute_flag == EXEC_NONLINEAR || _current_execute_flag == EXEC_LINEAR)
    {
      file_name << '_' << std::setw(_padding) << std::setprecision(0) << std::setfill('0')
                << std::right << _nonlinear_iter;
    }
    if (_current_execute_flag == EXEC_LINEAR)
    {
      file_name << '_' << std::setw(_padding) << std::setprecision(0) << std::setfill('0')
                << std::right << _linear_iter;
    }
  }

  file_name << ".csv";

  if (is_distributed)
  {
    int digits = MooseUtils::numDigits(n_processors());
    file_name << "." << std::setw(digits) << std::setfill('0') << processor_id();
  }
  return file_name.str();
}

void
CSV::output()
{
  // Call the base class output (populates tables)
  TableOutput::output();

  // Print the table containing all the data to a file
  if (_write_all_table && !_all_data_table.empty() && processor_id() == 0)
  {
    if (_sort_columns)
      _all_data_table.sortColumns();
    _all_data_table.printCSV(filename(), 1, _align);
  }

  // Output each VectorPostprocessor's data to a file
  if (_write_vector_table)
  {
    // The VPP table will not write the same data twice, so to get the symlinks correct
    // for EXEC_FINAL (when other flags exist) whenever files are written the names must
    // be stored. These stored names are then used outside of this loop when the EXEC_FINAL call is
    // made.
    _latest_vpp_filenames.clear();

    for (auto & it : _vector_postprocessor_tables)
    {
      const auto & vpp_name = it.first;
      it.second.setDelimiter(_delimiter);
      it.second.setPrecision(_precision);
      if (_sort_columns)
        it.second.sortColumns();

      bool include_time_suffix = true;
      bool is_distributed = _reporter_data.hasReporterWithMode(vpp_name, REPORTER_MODE_DISTRIBUTED);
      if (hasVectorPostprocessorByName(vpp_name))
      {
        const VectorPostprocessor & vpp_obj =
            _problem_ptr->getVectorPostprocessorObjectByName(vpp_name);
        include_time_suffix = !vpp_obj.containsCompleteHistory();
      }

      if (is_distributed || processor_id() == 0)
      {
        std::string fname =
            getVectorPostprocessorFileName(vpp_name, include_time_suffix, is_distributed);
        std::string fprefix = getVectorPostprocessorFilePrefix(vpp_name);

        _latest_vpp_filenames.emplace_back(fname, fprefix, is_distributed);

        it.second.printCSV(fname, 1, _align);

        if (_create_latest_symlink)
        {
          std::ostringstream out_latest;
          out_latest << fprefix << "_LATEST.csv";
          if (is_distributed)
          {
            int digits = MooseUtils::numDigits(n_processors());
            out_latest << "." << std::setw(digits) << std::setfill('0') << processor_id();
          }
          MooseUtils::createSymlink(fname, out_latest.str());
        }

        if (_time_data)
          _vector_postprocessor_time_tables[vpp_name].printCSV(fprefix + "_time.csv");
      }
    }
  }

  if (_current_execute_flag == EXEC_FINAL && _create_final_symlink)
  {
    for (const auto & name_tuple : _latest_vpp_filenames)
    {
      std::ostringstream out_final;
      out_final << std::get<1>(name_tuple) << "_FINAL.csv";
      if (std::get<2>(name_tuple))
      {
        int digits = MooseUtils::numDigits(n_processors());
        out_final << "." << std::setw(digits) << std::setfill('0') << processor_id();
        MooseUtils::createSymlink(std::get<0>(name_tuple), out_final.str());
      }
      else if (processor_id() == 0)
        MooseUtils::createSymlink(std::get<0>(name_tuple), out_final.str());
    }
  }

  // Re-set write flags
  _write_all_table = false;
  _write_vector_table = false;
}

std::string
CSV::getVectorPostprocessorFilePrefix(const std::string & vpp_name)
{
  return _file_base + "_" + MooseUtils::shortName(vpp_name);
}
