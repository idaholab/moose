//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GMVOUTPUT_H
#define GMVOUTPUT_H

// MOOSE includes
#include "OversampleOutput.h"

// Forward declarations
class GMVOutput;

template <>
InputParameters validParams<GMVOutput>();

/**
 * Class for output data to the GMVOutputII format
 */
class GMVOutput : public OversampleOutput
{
public:
  /**
   * Class constructor
   */
  GMVOutput(const InputParameters & parameters);

protected:
  /**
   * Overload the Output::output method, this is required for GMVOutput
   * output due to the method utilized for outputing
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
};

#endif /* GMVOUTPUT_H */
