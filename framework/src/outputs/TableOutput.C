//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TableOutput.h"

// MOOSE includes
#include "Conversion.h"
#include "FEProblem.h"
#include "Executioner.h"
#include "MooseApp.h"
#include "MooseVariableScalar.h"
#include "PetscSupport.h"
#include "Postprocessor.h"
#include "SystemBase.h"

#include "libmesh/dof_map.h"
#include "libmesh/string_to_enum.h"

InputParameters
TableOutput::validParams()
{
  // Base class parameters
  InputParameters params = AdvancedOutput::validParams();
  params += AdvancedOutput::enableOutputTypes("postprocessor scalar vector_postprocessor reporter");

  // Option for writing vector_postprocessor time file
  params.addParam<bool>("time_data",
                        false,
                        "When true and VectorPostprocessor data exists, write "
                        "a csv file containing the timestep and time "
                        "information.");

  // Add option for appending file on restart
  params.addParam<bool>("append_restart", false, "Append existing file on restart");

  params.addParam<bool>(
      "time_column",
      true,
      "Whether or not the 'time' column should be written for Postprocessor CSV files");

  params.addParam<Real>("new_row_tolerance",
                        libMesh::TOLERANCE * libMesh::TOLERANCE,
                        "The independent variable tolerance for determining when a new row should "
                        "be added to the table (Note: This value must be set independently for "
                        "Postprocessor output to type=Console and type=CSV file separately.");
  params.addParamNamesToGroup("new_row_tolerance time_data time_column", "Table formatting");

  return params;
}

TableOutput::TableOutput(const InputParameters & parameters)
  : AdvancedOutput(parameters),
    _tables_restartable(getParam<bool>("append_restart")),
    _postprocessor_table(_tables_restartable
                             ? declareRestartableData<FormattedTable>("postprocessor_table")
                             : declareRecoverableData<FormattedTable>("postprocessor_table")),
    _vector_postprocessor_time_tables(
        _tables_restartable ? declareRestartableData<std::map<std::string, FormattedTable>>(
                                  "vector_postprocessor_time_table")
                            : declareRecoverableData<std::map<std::string, FormattedTable>>(
                                  "vector_postprocessor_time_table")),
    _scalar_table(_tables_restartable ? declareRestartableData<FormattedTable>("scalar_table")
                                      : declareRecoverableData<FormattedTable>("scalar_table")),
    _reporter_table(_tables_restartable ? declareRestartableData<FormattedTable>("reporter_table")
                                        : declareRecoverableData<FormattedTable>("reporter_table")),
    _all_data_table(_tables_restartable ? declareRestartableData<FormattedTable>("all_data_table")
                                        : declareRecoverableData<FormattedTable>("all_data_table")),
    _new_row_tol(getParam<Real>("new_row_tolerance")),
    _time_data(getParam<bool>("time_data")),
    _time_column(getParam<bool>("time_column"))

{
  // Set a Boolean indicating whether or not we will output the time column
  _reporter_table.outputTimeColumn(_time_column);
  _postprocessor_table.outputTimeColumn(_time_column);
  _all_data_table.outputTimeColumn(_time_column);
}

void
TableOutput::outputPostprocessors()
{
  // Add new row to the tables
  if (_postprocessor_table.empty() ||
      !MooseUtils::absoluteFuzzyEqual(_postprocessor_table.getLastTime(), time(), _new_row_tol))
    _postprocessor_table.addRow(time());

  if (_all_data_table.empty() ||
      !MooseUtils::absoluteFuzzyEqual(_all_data_table.getLastTime(), time(), _new_row_tol))
    _all_data_table.addRow(time());

  // List of names of the postprocessors to output
  const std::set<std::string> & out = getPostprocessorOutput();

  // Loop through the postprocessor names and extract the values from the PostprocessorData storage
  for (const auto & out_name : out)
  {
    const PostprocessorValue & value = _problem_ptr->getPostprocessorValueByName(out_name);
    _postprocessor_table.addData(out_name, value);
    _all_data_table.addData(out_name, value);
  }
}

void
TableOutput::outputReporters()
{
  // List of VPP objects with output
  const std::set<std::string> & vpps = getVectorPostprocessorOutput();

  for (const auto & combined_name : getReporterOutput())
  {
    ReporterName r_name(combined_name);

    outputReporter<bool>(r_name);
    outputReporter<unsigned short int>(r_name);
    outputReporter<unsigned int>(r_name);
    outputReporter<unsigned long int>(r_name);
    outputReporter<unsigned long long int>(r_name);
    outputReporter<short int>(r_name);
    outputReporter<int>(r_name);
    outputReporter<long int>(r_name);
    outputReporter<long long int>(r_name);
    outputReporter<float>(r_name);
    outputReporter<long double>(r_name);
    outputReporter<char>(r_name);
    outputReporter<std::string>(r_name);

    // Need to check for reals because PPs and VPPs might have already been outputted
    if (!hasPostprocessorByName(r_name.getObjectName()) &&
        vpps.find(r_name.getObjectName()) == vpps.end())
      outputReporter<Real>(r_name);
  }
}

void
TableOutput::outputVectorPostprocessors()
{
  // List of VPP objects with output
  const std::set<std::string> & out = getVectorPostprocessorOutput();

  for (const auto & r_name : _reporter_data.getReporterNames())
  {
    const std::string & vpp_name = r_name.getObjectName();
    const std::string & vec_name = r_name.getValueName();
    const bool vpp_out = out.find(vpp_name) != out.end();
    if (vpp_out && (_reporter_data.hasReporterValue<VectorPostprocessorValue>(r_name)))
    {
      auto insert_pair =
          moose_try_emplace(_vector_postprocessor_tables, vpp_name, FormattedTable());

      FormattedTable & table = insert_pair.first->second;
      table.outputTimeColumn(false);

      const auto & vector = _reporter_data.getReporterValue<VectorPostprocessorValue>(r_name);
      table.addData(vec_name, vector);

      if (_time_data)
      {
        FormattedTable & t_table = _vector_postprocessor_time_tables[vpp_name];
        t_table.addData("timestep", _t_step, _time);
      }
    }
  }
}

void
TableOutput::outputScalarVariables()
{
  // List of scalar variables
  const std::set<std::string> & out = getScalarOutput();

  // Loop through each variable
  for (const auto & out_name : out)
  {
    // Get reference to the variable (0 is for TID)
    MooseVariableScalar & scalar_var = _problem_ptr->getScalarVariable(0, out_name);

    // Make sure the value of the variable is in sync with the solution vector
    scalar_var.reinit();

    // Next we need to make sure all processors agree on the value of
    // the variable - not all processors may be able to see all
    // scalars!

    // Make a copy rather than taking a reference to the MooseArray,
    // because if a processor can't see that scalar variable's values
    // then we'll need to do our own communication of them.
    VariableValue value(scalar_var.sln());
    auto value_size = value.size();

    // libMesh *does* currently guarantee that all processors can
    // calculate all scalar DoF indices, so this is a const reference
    const std::vector<dof_id_type> & dof_indices = scalar_var.dofIndices();
    auto dof_size = dof_indices.size();
    bool need_release = false;

    // In dbg mode, if we don't see a scalar we might not even have
    // its array allocated to full length yet.
    if (dof_size > value_size)
    {
      value.resize(dof_size);
      need_release = true;
    }

    // Finally, let's just let the owners broadcast their values.
    // There's probably lots of room to optimize this communication
    // via merging broadcasts and making them asynchronous, but this
    // code path shouldn't be hit often enough for that to matter.

    const DofMap & dof_map = scalar_var.sys().dofMap();
    for (decltype(dof_size) i = 0; i < dof_size; ++i)
    {
      const processor_id_type pid = dof_map.dof_owner(dof_indices[i]);
      this->comm().broadcast(value[i], pid);
    }

    // If the variable has a single component, simply output the value with the name
    if (dof_size == 1)
    {
      _scalar_table.addData(out_name, value[0], time());
      _all_data_table.addData(out_name, value[0], time());
    }

    // Multi-component variables are appended with the component index
    else
      for (decltype(dof_size) i = 0; i < dof_size; ++i)
      {
        std::ostringstream os;
        os << out_name << "_" << i;
        _scalar_table.addData(os.str(), value[i], time());
        _all_data_table.addData(os.str(), value[i], time());
      }

    // If we ended up reallocating, we'll need to release memory or leak it
    if (need_release)
      value.release();
  }
}

void
TableOutput::clear()
{
  _reporter_table.clear();
  _postprocessor_table.clear();
  for (auto & pair : _vector_postprocessor_tables)
    pair.second.clear();
  for (auto & pair : _vector_postprocessor_time_tables)
    pair.second.clear();
  _scalar_table.clear();
  _all_data_table.clear();
}
