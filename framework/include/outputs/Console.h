/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef CONSOLE_H
#define CONSOLE_H

// MOOSE includes
#include "TableOutputter.h"
#include "FileOutputInterface.h"
#include "FormattedTable.h"
#include "Conversion.h"

// libMesh includes
#include "libmesh/string_to_enum.h"

// Forward declerations
class Console;

template<>
InputParameters validParams<Console>();

/**
 * An output object for writting to the console (screen)
 */
class Console :
  public TableOutputter,
  public FileOutputInterface
{
public:

  /**
   * Class constructor
   */
  Console(const std::string & name, InputParameters);

  /**
   * Destructor
   */
  virtual ~Console();

  /**
   * Initial setup function
   * Prints the system information, this is done here so that the system information
   * is printed prior to any PETSc solve information
   */
  virtual void initialSetup();

  /**
   * Timestep function
   * Runs at the beginning of each timestep and prints the timestep, time, and dt information
   */
  virtual void timestepSetup();

  /**
   * Adds a outputting of nonlinear/linear reisdual printing to the base class output() method
   *
   * @see petscOutput
   */
  virtual void output();

  /**
   * Creates the output file name
   * Appends the user-supplied 'file_base' input parameter with a '.txt' extension
   * @return A string containg the output filename
   */
  virtual std::string filename();

  /**
   * A helper function for printing linear residuals via PETSc
   * @param its A reference to the iteration numbe from PETSc
   * @param norm A reference to the linear residual norm from PETSc
   * @see PetscSupport::petscLinearMonitor
   */
  void linearMonitor(PetscInt & its, PetscReal & norm);

  /**
   * A helper function for printing linear residuals via PETSc
   * @param its A reference to the iteration numbe from PETSc
   * @param norm A reference to the linear residual norm from PETSc
   * @see PetscSupport::petscLinearMonitor
   */
  void nonlinearMonitor(PetscInt & its, PetscReal & norm);

  /**
   * Display the system information
   */
  void outputSystemInformation();

  /**
   * Output string for setting up PETSC output
   */
  static void petscSetupOutput();

protected:

  /**
   * Prints the aux scalar variables table to the screen
   */
  virtual void outputScalarVariables();

  /**
   * Prints the postprocessor table to the screen
   */
  virtual void outputPostprocessors();

  /**
   * Setups up PETSc to output the nonlinear/linear residuals
   */
   virtual void petscSetup();

  /**
   * A helper function for outputing norms in color
   * @param old_norm The old residual norm to compare against
   * @param norm The current residual norm
   */
  std::string outputNorm(Real old_norm, Real norm);

  /** Helper function function for stringstream formating
   * @see outputSimulationInformation()
   */
  void insertNewline(std::stringstream &oss, std::streampos &begin, std::streampos &curr);

  /**
   * Write the file stream to the file
   * This helper function writes the _file_output_stream to the file and clears the
   * stream, by default the file is appended.
   * @param append Toggle for appending the file
   */
  void writeStream(bool append = true);

  /// The max number of table rows
  unsigned int _max_rows;

  /// The FormattedTable fit mode
  MooseEnum _fit_mode;

  /// Toggle for controlling the use of color output
  bool _use_color;

  /// Toggle for controlling the printing of linear residuals (requires _print_nonlinear = true)
  bool _print_linear;

  /// Toggle for controlling the printing of nonlinear residuals
  bool _print_nonlinear;

  /// Flag for controlling outputing console information to a file
  bool _write_file;

  /// Flag for controlling outputing console information to screen
  bool _write_screen;

  /// Flag for writing detailed time step information
  bool _verbose;

  /// Stream for storing information to be written to a file
  std::stringstream _file_output_stream;

  /// Storage for the old linear residual (needed for color output)
  Real _old_linear_norm;

  /// Storage for the old non linear residual (needed for color output)
  Real _old_nonlinear_norm;

  /// State for all performance logging
  bool _perf_log;

  /// State for solve performace log
  bool _solve_log;

  /// State for setup performance log
  bool _setup_log;

  /// State for early setup log printing
  bool _setup_log_early;

  /// State for the performance log header information
  bool _perf_header;

  /// Width used for printing simulation information
  static const unsigned int _field_width = 25;

  /// Line length for printing simulation information
  static const unsigned int _line_length = 100;
};

#endif /* CONSOLE_H */
