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
#include "TableOutput.h"

/**
 * Based class for adding basic filename support to output base class
 *
 * @see Exodus
 */
class CSV : public TableOutput
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   *
   * The constructor performs all of the necessary initialization of the various
   * output lists required for the various output types.
   *
   * @see initAvailable init separate
   */
  CSV(const InputParameters & parameters);

protected:
  /**
   * Output the table to a *.csv file
   */
  virtual void output(const ExecFlagType & type) override;

  /**
   * The filename for the output file
   * @return A string of output file including the extension
   */
  virtual std::string filename() override;

  /**
   * Setup the CSV output
   * If restarting and the append_restart flag is false, then the output data is cleared here
   */
  void initialSetup() override;

  /**
   * Sets the write flag and calls TableOutput::outputScalarVariables()
   */
  virtual void outputScalarVariables() override;

  /**
   * Sets the write flag and calls TableOutput::outputPostprocessors()
   */
  virtual void outputPostprocessors() override;

  /**
   * Sets the write flag and calls TableOutput::outputVectorPostprocessors()
   */
  virtual void outputVectorPostprocessors() override;

  /**
   * Sets the write flag and calls TableOutput::outputVectorPostprocessors()
   */
  virtual void outputReporters() override;

  /**
   * Generates a filename pattern for Vectorpostprocessors
   * filebase + VPP name + time step + ".csv"
   */
  std::string getVectorPostprocessorFileName(const std::string & vpp_name,
                                             bool include_time_step,
                                             bool is_distributed);

private:
  /// Flag for aligning data in .csv file
  bool _align;

  /// Decimal digits per number in the CSV file
  unsigned int _precision;

  /// The delimiter used when writing the CSV file
  std::string _delimiter;

  /// Flag for writing scalar and/or postprocessor data
  bool _write_all_table;

  /// Flag for writing vector postprocessor data
  bool _write_vector_table;

  /// Flag for sorting column names
  const bool _sort_columns;

  /// Flag indicating MOOSE is recovering via --recover command-line option
  bool _recovering;

  /// Flag for creating a _FINAL symlink
  bool _create_final_symlink;

  /// Flag for creating a _LATEST symlink
  bool _create_latest_symlink;

  /// Current list of VPP filenames for creating _LATEST/_FINAL symlinks
  // The pair is composed of the complete filename (foo_variable_0001.csv), the incomplete name
  // (foo_variable) to which the _FINAL or _LATEST is to be applied, and the "is_distributed" flag
  std::vector<std::tuple<std::string, std::string, bool>> _latest_vpp_filenames;

  /**
   * Returns the filename without the time/timestep information.
   */
  std::string getVectorPostprocessorFilePrefix(const std::string & vpp_name);
};
