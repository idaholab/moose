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
class Gnuplot : public TableOutput
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   *
   * The constructor performs all of the necessary initialization of the various
   * output lists required for the various output types.
   *
   * @see initAvailable init seperate
   */
  Gnuplot(const InputParameters & parameters);

  /**
   * Output the table to a *.csv file
   */
  virtual void output(const ExecFlagType & type) override;

  /**
   * The filename for the output file
   * @return A string of output file including the extension
   */
  virtual std::string filename() override;

private:
  /// Desired file extension
  std::string _extension;
};
