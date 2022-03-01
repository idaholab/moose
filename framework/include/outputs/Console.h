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
 * An output object for writing to the console (screen)
 */
class Console : public TableOutput
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   */
  Console(const InputParameters & parameters);

  /**
   * Destructor
   */
  virtual ~Console();

  /**
   * Initial setup function
   * Prints the system information, this is done here so that the system information
   * is printed prior to any PETSc solve information
   */
  virtual void initialSetup() override;

  virtual void timestepSetup() override;

  /**
   * Customizes the order of output for the various components as well as adds additional
   * output such as timestep information and nonlinear/linear residual information
   *
   * This method explicitly re-implements portions of AdvancedOutput::output, which is generally not
   * recommended. This is done here to get the output ordering desired. If additional output types
   * (e.g., elemental or nodal) are required in the future this calls will need to be explicitly
   * added
   * as well.
   */
  virtual void output(const ExecFlagType & type) override;

  /**
   * Creates the output file name
   * Appends the user-supplied 'file_base' input parameter with a '.txt' extension
   * @return A string containing the output filename
   */
  virtual std::string filename() override;

  /**
   * Output string for setting up PETSC output
   */
  static void petscSetupOutput();

  /**
   * Performs console related printing when the mesh is changed
   */
  void meshChanged() override;

  /**
   * A helper function for outputting norms in color
   * @param old_norm The old residual norm to compare against
   * @param norm The current residual norm
   */
  static std::string
  outputNorm(const Real & old_norm, const Real & norm, const unsigned int precision = 6);

  /**
   * Return system information flags
   */
  MultiMooseEnum & systemInfoFlags()
  {
    if (!_allow_changing_sysinfo_flag)
      mooseError(
          "accessing console system information flags is not allowed after console initial setup");
    return _system_info_flags;
  }

protected:
  /**
   * Print the input file at the beginning of the simulation
   */
  virtual void outputInput() override;

  /**
   * Prints the aux scalar variables table to the screen
   */
  virtual void outputScalarVariables() override;

  /**
   * Prints the postprocessor table to the screen
   */
  virtual void outputPostprocessors() override;

  /**
   * Not implemented.
   */
  virtual void outputVectorPostprocessors() override
  {
    mooseError("Can't currently output VectorPostprocessors to the screen");
  };

  /**
   * Prints the Reporter values to the screen
   */
  virtual void outputReporters() override;

  /**
   * Print system information
   */
  virtual void outputSystemInformation() override;

  /**
   * Prints the time step information for the screen output. The Boolean controls whether the dt is
   * output. It doesn't really make sense to output this quantity the first time since it's a delta
   * quantity indicating the step size from the previous step.
   */
  void writeTimestepInformation(bool output_dt);

  /**
   * Write message to screen and/or file
   * @param message The desired message
   * @param indent True if multiapp indenting is desired
   */
  void write(std::string message, bool indent = true);

  /**
   * Write the file stream to the file
   * @param append Toggle for appending the file
   *
   * This helper function writes the _file_output_stream to the file and clears the
   * stream, by default the file is appended. This does nothing if 'output_file' is
   * false.
   */
  void writeStreamToFile(bool append = true);

  /**
   * Print the L2-norms for each variable
   */
  void writeVariableNorms();

  /// The max number of table rows
  unsigned int _max_rows;

  /// The FormattedTable fit mode
  MooseEnum _fit_mode;

  /// Toggle for outputting time in time and dt in scientific notation
  bool _scientific_time;

  /// Flag for controlling outputting console information to a file
  bool _write_file;

  /// Flag for controlling outputting console information to screen
  bool _write_screen;

  /// Flag for writing detailed time step information
  bool _verbose;

  /// Stream for storing information to be written to a file
  std::stringstream _file_output_stream;

  /// State for all performance logging
  bool _perf_log;

  /// The interval at which the performance log is printed
  unsigned int _perf_log_interval;

  /// State for solve performance log
  bool _solve_log;

  /// Control the display libMesh performance log
  bool _libmesh_log;

  /// State for the performance log header information
  bool _perf_header;

  /// Flag for writing all variable norms
  bool _all_variable_norms;

  /// Flag for writing outlier variable norms
  bool _outlier_variable_norms;

  /// Multipliers for coloring variable residual norms (default [2, 0.8])
  std::vector<Real> _outlier_multiplier;

  /// Number of significant digits
  unsigned int _precision;

private:
  /**
   * Add a message to the output streams
   * @param message The message to add to the output streams
   *
   * Any call to this method will write the supplied message to the screen and/or file,
   * following the same restrictions as outputStep.
   *
   * Calls to this method should be made via OutputWarehouse::mooseConsole so that the
   * output stream buffer is cleaned up correctly. Thus, it is a private method.
   */
  void mooseConsole(const std::string & message);

  /// Reference to cached messages from calls to _console
  const std::ostringstream & _console_buffer;

  /// Storage for the old linear residual (needed for color output and only when used when printing to the screen)
  Real _old_linear_norm;

  /// Storage for the old non linear residual (needed for color output and only when used when printing to the screen)
  Real _old_nonlinear_norm;

  /// Flag for printing mesh information when the mesh changes
  bool _print_mesh_changed_info;

  /// Flags for controlling the what simulations information is shown
  MultiMooseEnum _system_info_flags;

  friend class OutputWarehouse;

private:
  /// A boolean for protecting _system_info_flags from being changed undesirably
  bool _allow_changing_sysinfo_flag;

  bool _last_message_ended_in_newline;
};
