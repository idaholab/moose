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
#include "TableOutputBase.h"
#include "FormattedTable.h"

// Forward declerations
class Console;

template<>
InputParameters validParams<Console>();

/**
 * An output object for writting to the console (screen)
 */
class Console :
  public TableOutputBase
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
   * Adds a outputting of nonlinear/linear reisdual printing to the base class output() method
   *
   * @see petscOutput
   */
  virtual void output();

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

private:

  /**
   * Setups up PETSc to output the nonlinear/linear residuals
   */
  void petscOutput();

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

  /// Toggle for controlling the printing of the system information
  bool _system_information;

};

#endif /* CONSOLE_H */
