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
#include "OversampleOutput.h"

/**
 * Class for output data to the TecplotII format
 */
class Tecplot : public OversampleOutput
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   */
  Tecplot(const InputParameters & parameters);

protected:
  /**
   * Overload the Output::output method, this is required for Tecplot
   * output due to the method utilized for outputting single/global parameters
   */
  virtual void output(const ExecFlagType & type) override;

  /**
   * Returns the current filename, this method handles adding the timestep suffix
   * @return A string containing the current filename to be written
   */
  virtual std::string filename() override;

private:
  /// Flag for binary output
  bool _binary;

  /// Flag for turning on appending to ASCII files
  bool _ascii_append;

  /// True if this is the first time the file has been written to,
  /// gets set to false after the first call to output().  If the user
  /// has set _ascii_append but _first_time==true, we won't actually
  /// append.  This prevents old data files in a directory from being
  /// appended to.  Declared as a reference so it can be restartable
  /// data, that way if we restart, we don't think it's the first time
  /// again.
  bool & _first_time;
};
