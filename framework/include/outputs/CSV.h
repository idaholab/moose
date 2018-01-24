//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CSV_H
#define CSV_H

// MOOSE includes
#include "TableOutput.h"

// Forward declarations
class CSV;

template <>
InputParameters validParams<CSV>();

/**
 * Based class for adding basic filename support to output base class
 *
 * @see Exodus
 */
class CSV : public TableOutput
{
public:
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

private:
  /// Flag for aligning data in .csv file
  bool _align;

  /// Decimal digits per number in the CSV file
  unsigned int _precision;

  /// The delimiter used when writing the CSV file
  std::string _delimiter;

  /// Flag for writting scalar and/or postprocessor data
  bool _write_all_table;

  /// Flag for writting vector postprocessor data
  bool _write_vector_table;

  /// Flag for sorting column names
  const bool _sort_columns;

  /// Flag indicating MOOSE is recovering via --recover command-line option
  bool _recovering;
};

#endif /* CSV_H */
