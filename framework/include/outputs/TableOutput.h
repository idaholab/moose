//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "AdvancedOutput.h"
#include "FileOutput.h"
#include "FormattedTable.h"

/**
 * Base class for scalar variables and postprocessors output objects
 *
 * This class populates three FormattedTable objects that may then be used
 * by child classes for creating custom output objects:
 * _all_data_table - includes the data from both postprocessors and scalar aux variables
 * _postprocessor_table - includes the data from only the postprocessors
 * _scalar_table - includes the data from only the scalar aux variables
 *
 * @see CSV Console
 */
class TableOutput : public AdvancedOutput
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   */
  TableOutput(const InputParameters & parameters);

  void clear();

protected:
  /**
   * Populates the tables with scalar aux variables
   *
   * If an aux variable contains multiple components the output name for the
   * variable is appended with the component number (e.g., aux_0, aux_1, ...)
   */
  virtual void outputScalarVariables() override;

  /**
   * Populates the tables with postprocessor values
   */
  virtual void outputPostprocessors() override;

  /**
   * Populates the tables with Reporter values
   */
  virtual void outputReporters() override;
  template <typename T>
  void outputReporter(const ReporterName & name);

  /**
   * Populates the tables with VectorPostprocessor values
   */
  virtual void outputVectorPostprocessors() override;

  /// Flag for allowing all table data to become restartable
  bool _tables_restartable;

  /// Table containing postprocessor data
  FormattedTable & _postprocessor_table;

  /// Formatted tables for outputting vector postprocessor data.  One per VectorPostprocessor
  std::map<std::string, FormattedTable> _vector_postprocessor_tables;

  /// Table for vector postprocessor time data
  std::map<std::string, FormattedTable> & _vector_postprocessor_time_tables;

  /// Table containing scalar aux variables
  FormattedTable & _scalar_table;

  /// Table containing Real Reporter values
  FormattedTable & _reporter_table;

  /// Table containing postprocessor values, scalar aux variables, and Real Reporters
  FormattedTable & _all_data_table;

  /// Tolerance used when deciding whether or not to add a new row to the table
  const Real _new_row_tol;

  /// Enable/disable VecptorPostprocessor time data file.
  const bool _time_data;

  /// Enable/disable output of time column for Postprocessors
  const bool _time_column;
};

template <typename T>
void
TableOutput::outputReporter(const ReporterName & name)
{
  static_assert(TableValueBase::isSupportedType<T>(), "Unsupported table value type.");

  if (_reporter_data.hasReporterValue<T>(name))
  {
    if (_reporter_table.empty() ||
        !MooseUtils::absoluteFuzzyEqual(_reporter_table.getLastTime(), time(), _new_row_tol))
      _reporter_table.addRow(time());

    if (_all_data_table.empty() ||
        !MooseUtils::absoluteFuzzyEqual(_all_data_table.getLastTime(), time(), _new_row_tol))
      _all_data_table.addRow(time());

    const T & value = _reporter_data.getReporterValue<T>(name);
    _reporter_table.addData<T>(name.getCombinedName(), value);
    _all_data_table.addData<T>(name.getCombinedName(), value);
  }
  else if (_reporter_data.hasReporterValue<std::vector<T>>(name))
  {
    const std::string & obj_name = name.getObjectName();
    const std::string & val_name = name.getValueName();
    auto insert_pair = moose_try_emplace(_vector_postprocessor_tables, obj_name, FormattedTable());

    FormattedTable & table = insert_pair.first->second;
    table.outputTimeColumn(false);

    const std::vector<T> & vector = _reporter_data.getReporterValue<std::vector<T>>(name);
    table.addData<T>(val_name, vector);

    if (_time_data)
    {
      FormattedTable & t_table = _vector_postprocessor_time_tables[obj_name];
      t_table.addData("timestep", _t_step, _time);
    }
  }
}
