//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * Class for output data to the XDAII format
 */
class XDA : public OversampleOutput
{
public:
  static InputParameters validParams();

  /**
   * Class consturctor
   */
  XDA(const InputParameters & parameters);

  bool supportsMaterialPropertyOutput(bool /* suppress_warnings */) const override { return true; }

protected:
  /**
   * Overload the Output::output method, this is required for XDA
   * output due to the method utlized for outputting single/global parameters
   */
  virtual void output() override;

  /**
   * Returns the current filename, this method handles adding the timestep suffix
   * @return A string containg the current filename to be written
   */
  virtual std::string filename() override;

private:
  /// Flag for binary output
  bool _binary;
};
